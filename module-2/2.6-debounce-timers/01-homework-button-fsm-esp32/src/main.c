#include <inttypes.h>
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button_fsm.h"

#define BUTTON_GPIO GPIO_NUM_4
#define LED_GPIO GPIO_NUM_5
#define POLL_MS 10
static const char *TAG = "button";
static bool led_state;

static void set_led(bool on) {
    led_state = on;
    ESP_ERROR_CHECK(gpio_set_level(LED_GPIO, led_state));
}

// One callback per event. Each gets the event and its ctx; these four need neither.
static void on_short(button_event_t ev, void *ctx) {
    (void)ev; (void)ctx;
    ESP_LOGI(TAG, "SHORT: toggle LED");
    set_led(!led_state);
}

static void on_double(button_event_t ev, void *ctx) {
    (void)ev; (void)ctx;
    ESP_LOGI(TAG, "DOUBLE: LED off");
    set_led(false);
}

static void on_long(button_event_t ev, void *ctx) {
    (void)ev; (void)ctx;
    ESP_LOGI(TAG, "LONG: LED on");
    set_led(true);
}

static void on_stuck(button_event_t ev, void *ctx) {
    (void)ev; (void)ctx;
    ESP_LOGW(TAG, "STUCK > 7s: check input!");
    set_led(false);
}

void app_main(void) {
    gpio_config_t in = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config_t out = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&in));
    ESP_ERROR_CHECK(gpio_config(&out));
    ESP_ERROR_CHECK(gpio_set_level(LED_GPIO, 0));

    button_fsm_t button;
    button_fsm_init(&button, esp_timer_get_time());
    // The device decides what each gesture does; after init, which clears them
    button_fsm_on(&button, BUTTON_SHORT,  on_short,  NULL);
    button_fsm_on(&button, BUTTON_DOUBLE, on_double, NULL);
    button_fsm_on(&button, BUTTON_LONG,   on_long,   NULL);
    button_fsm_on(&button, BUTTON_STUCK,  on_stuck,  NULL);
    const TickType_t interval = pdMS_TO_TICKS(POLL_MS);
    TickType_t last_wake = xTaskGetTickCount();
    ESP_LOGI(TAG, "GPIO4 -> button -> 3.3V; GPIO5 -> 330R -> LED -> GND");
    while (true) {
        bool raw = gpio_get_level(BUTTON_GPIO) == 1;
        button_fsm_poll(&button, raw, esp_timer_get_time());
        xTaskDelayUntil(&last_wake, interval);
    }
}
