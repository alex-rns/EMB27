#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct
{
	gpio_num_t pin;
	int64_t half_period;             // ms between toggles: a full blink takes twice this
	int64_t last_change_timestamp;   // ms, from esp_timer_get_time() / 1000
	uint32_t level;                  // our own copy: no TogglePin here, and gpio_get_level() reads 0 on an output-only pin
} LED_type_t;

LED_type_t LEDS[3] =
{
	[0] = {.pin = GPIO_NUM_6, .level = 0, .last_change_timestamp = 0, .half_period = 200},
	[1] = {.pin = GPIO_NUM_4, .level = 0, .last_change_timestamp = 0, .half_period = 500},
	[2] = {.pin = GPIO_NUM_5, .level = 0, .last_change_timestamp = 0, .half_period = 1000},
};

void app_main(void) {
    for (uint8_t led_idx = 0; led_idx < 3; led_idx++) {
        gpio_reset_pin(LEDS[led_idx].pin);
        gpio_set_direction(LEDS[led_idx].pin, GPIO_MODE_OUTPUT);
    }

    while (true) {
        int64_t now_ms = esp_timer_get_time() / 1000;   // us since boot -> ms; one timestamp for the whole pass

        for (uint8_t led_idx = 0; led_idx < 3; led_idx++) {
            LED_type_t* led = &LEDS[led_idx];   // pointer to the array element, not a copy

            if (now_ms - led->last_change_timestamp >= led->half_period) {
                led->level = !led->level;
                gpio_set_level(led->pin, led->level);
                led->last_change_timestamp = now_ms;
            }
        }
    }
}
