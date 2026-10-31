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
    gpio_reset_pin(GPIO_NUM_7);                          // plain GPIO function, input and output off
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_INPUT);     // input only: gpio_get_level() reads the pin
    gpio_set_pull_mode(GPIO_NUM_7, GPIO_PULLUP_ONLY);    // internal pull-up to 3.3 V: released = 1, pressed (to GND) = 0

    gpio_reset_pin(GPIO_NUM_15);                         // the interval button: set up like GPIO 7
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);

    uint32_t paused = 0;           // 1 = all LEDs frozen in their current state
    int last_btn2_state = 1;       // level on the previous pass; 1 = released (pull-up)
    int last_btn1_state = 1;       // GPIO 15 has its own memory, so one button never resets the other

    for (uint8_t led_idx = 0; led_idx < 3; led_idx++) {
        gpio_reset_pin(LEDS[led_idx].pin);
        gpio_set_direction(LEDS[led_idx].pin, GPIO_MODE_OUTPUT);
    }

    while (true) {
        int64_t now_ms = esp_timer_get_time() / 1000;   // us since boot -> ms; one timestamp for the whole pass

        int btn2_state = gpio_get_level(GPIO_NUM_7);    // GPIO 7 to GND: 0 = held

        // press edge: released last pass, held now; true on one pass per press
        if (last_btn2_state == 1 && btn2_state == 0) {
            paused = !paused;
        }
        last_btn2_state = btn2_state;   // every pass, outside the if

        int btn1_state = gpio_get_level(GPIO_NUM_15);

        // each press moves GPIO 5 (LEDS[2]) to the next interval: 1000 -> 500 -> 200 -> 1000
        if (last_btn1_state == 1 && btn1_state == 0) {
            if (LEDS[2].half_period == 1000) {   // the array element itself, so '.' and no pointer
                LEDS[2].half_period = 500;
            } else if (LEDS[2].half_period == 500) {
                LEDS[2].half_period = 200;
            } else {                             // 200 or anything unexpected: back to the start
                LEDS[2].half_period = 1000;
            }
        }
        last_btn1_state = btn1_state;

        for (uint8_t led_idx = 0; led_idx < 3 && !paused; led_idx++) {   // paused: no LED is touched
            LED_type_t* led = &LEDS[led_idx];   // pointer to the array element, not a copy

            if (now_ms - led->last_change_timestamp >= led->half_period) {
                led->level = !led->level;
                gpio_set_level(led->pin, led->level);
                led->last_change_timestamp = now_ms;
            }
        }
    }
}
