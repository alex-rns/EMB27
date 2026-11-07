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

static void on_button_event(button_event_t ev, void *ctx) {
    (void)ctx;
    switch (ev) {
    case BUTTON_SHORT:  led_state = !led_state; ESP_LOGI(TAG, "SHORT: toggle LED"); break;
    case BUTTON_DOUBLE: led_state = false; ESP_LOGI(TAG, "DOUBLE: LED off"); break;
    case BUTTON_LONG:   led_state = true;  ESP_LOGI(TAG, "LONG: LED on"); break;
    case BUTTON_STUCK:  led_state = false; ESP_LOGW(TAG, "STUCK > 7s: check input!"); break;
    }
    ESP_ERROR_CHECK(gpio_set_level(LED_GPIO, led_state));
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
    const TickType_t interval = pdMS_TO_TICKS(POLL_MS);
    TickType_t last_wake = xTaskGetTickCount();
    ESP_LOGI(TAG, "GPIO4 -> button -> 3.3V; GPIO5 -> 330R -> LED -> GND");
    while (true) {
        bool raw = gpio_get_level(BUTTON_GPIO) == 1;
        button_fsm_poll(&button, raw, esp_timer_get_time(), on_button_event, NULL);
        xTaskDelayUntil(&last_wake, interval);
    }
}
