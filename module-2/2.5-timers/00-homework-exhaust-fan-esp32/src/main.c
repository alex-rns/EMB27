#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"

#define RELAY_PIN GPIO_NUM_7

static bool is_relay_on = false;

static bool on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    is_relay_on = !is_relay_on;
    gpio_set_level(RELAY_PIN, is_relay_on);
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
        .alarm_count = 2000000,               // 2 s at 1 tick = 1 us
        .reload_count = 0,                    // the counter restarts from 0 at the alarm...
        .flags.auto_reload_on_alarm = true,   // ...by hardware, every time: a periodic alarm
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_enable(timer));
    ESP_ERROR_CHECK(gptimer_start(timer));
    // app_main returns here and its task ends; the timer keeps running and calling on_alarm()
}
