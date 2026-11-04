#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

#define RELAY_PIN GPIO_NUM_7
#define FAN_PERIOD_S  8   // full cycle; 3600 for 1 hour
#define FAN_ON_S      2   // fan on; 900 for 15 min

volatile static bool is_relay_on = false;

static bool on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    is_relay_on = !is_relay_on;
    gpio_set_level(RELAY_PIN, is_relay_on);

    // how long the new state lasts, in ticks: on for FAN_ON_S, off for the rest of the period
    uint64_t next_ticks = is_relay_on ? FAN_ON_S * 1000000ULL
                                      : (FAN_PERIOD_S - FAN_ON_S) * 1000000ULL;
    gptimer_alarm_config_t next_alarm = {
        .alarm_count = next_ticks,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer, &next_alarm);   // allowed inside the ISR: the next alarm comes after next_ticks

    return false;
}

void app_main(void)
{
    gpio_reset_pin(RELAY_PIN);                        // plain GPIO, old setup cleared (also enables the pull-up)
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);  // output driver on: the pin now drives 3.3 V or 0 V
    gpio_set_level(RELAY_PIN, 0);

    gptimer_handle_t timer = NULL;   // the driver writes the new timer here

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,        // which clock feeds the timer
        .direction = GPTIMER_COUNT_UP,      // count up or down
        .resolution_hz = 1000000,  // ticks per second
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));   // stops with a log message if the driver fails

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = on_alarm,   // called from the timer interrupt at every alarm
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &callbacks, NULL));   // must come before gptimer_enable()

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = (FAN_PERIOD_S - FAN_ON_S) * 1000000ULL, // off phase first: the relay starts off
        .reload_count = 0,                    // the counter restarts from 0 at the alarm...
        .flags.auto_reload_on_alarm = true,   // ...by hardware, every time: a periodic alarm
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_enable(timer));
    ESP_ERROR_CHECK(gptimer_start(timer));
    
    bool printed = is_relay_on;   // the state already reported

    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));   // watch this loop: no esp_task_wdt_reset() for 5 s -> the task watchdog fires
    while (true) {
        bool state = is_relay_on;   // one read per pass: the ISR may change it at any moment
        if (state != printed) {
            printf("fan %s\n", state ? "ON" : "OFF");
            printed = state;        // report each change once
        }
        vTaskDelay(pdMS_TO_TICKS(10));   // pause so the idle task feeds the task watchdog (as in 2.4)
        esp_task_wdt_reset();            // feed the watchdog: this loop is alive
    }
}
