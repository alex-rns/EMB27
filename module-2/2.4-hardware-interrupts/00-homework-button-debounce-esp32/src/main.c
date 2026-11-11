#include "driver/gpio.h"
#include "esp_attr.h"   // IRAM_ATTR
#include  <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"     // vTaskDelay
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_7  // button to GND, external 10 kOhm pull-up to 3V3

// Task 3 (interrupt), off in task 4:
// // Written by the ISR, read by the loop: volatile makes the loop re-read it from RAM every time
// static volatile uint32_t counter = 0;
//
// // Runs on every falling edge of BUTTON_PIN; IRAM_ATTR keeps it in RAM, not in flash
// IRAM_ATTR void button_isr(void *arg) {
//     counter++;
// }

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);                      // also turns the internal pull-up on
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_FLOATING);   // internal pull-up off: only the external 10 kOhm

    // Task 3 (interrupt), off in task 4:
    // gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);  // falling edge: 1 -> 0
    // gpio_install_isr_service(0);                         // once per program; must come before handler_add
    // gpio_isr_handler_add(BUTTON_PIN, button_isr, NULL);  // the function itself, no (); NULL arrives as arg

    static uint32_t press_count = 0;

    // Task 3 (interrupt), off in task 4:
    // static uint32_t last_edge_count = 0;
    // static int64_t last_edge_us = 0;
    // static bool event_pending = false;

    // Task 4: no interrupt, the loop polls the pin every 10 ms.
    // State machine with 2 states: stable = 1 (released) or 0 (pressed), switched only after 5 equal polls
    static int stable = 1;
    static int same_count = 0;      // polls in a row where the pin differs from stable

    while (true)
    {
        int level = gpio_get_level(BUTTON_PIN);  // 0 = pressed (button to GND), 1 = released

        if (level != stable) {             // the pin differs from the accepted state
            same_count++;
            if (same_count >= 5) {         // 5 polls x 10 ms = the new level held 50 ms
                stable = level;            // switch state: released <-> pressed
                same_count = 0;
                if (stable == 0) {         // switched to pressed: count it
                    press_count++;
                    printf("press: %lu\n", press_count); // uint32_t is unsigned long here: %lu
                }
            }
        } else {
            same_count = 0;                // bounce went back: start counting again
        }

        // Task 3 (interrupt), off in task 4:
        // uint32_t now_counter = counter; // read once: the ISR may change counter at any moment
        //
        // // new edges: remember when the last one came; the button is checked later
        // if (now_counter != last_edge_count)
        // {
        //     last_edge_count = now_counter;
        //     last_edge_us = esp_timer_get_time(); // microseconds since boot
        //     event_pending = true;
        // }
        //
        // // state-based debounce: 50 ms after the last edge, it is a press only if the button is still down
        // if (event_pending && esp_timer_get_time() - last_edge_us >= 50000)
        // {
        //     event_pending = false;
        //     if (gpio_get_level(BUTTON_PIN) == 0) // 0 = pressed (button to GND); 1 = the edges were a release
        //     {
        //         press_count++;
        //         printf("press: %lu (edges: %lu)\n", press_count, now_counter); // uint32_t is unsigned long here: %lu
        //     }
        // }

        // = Arduino delay(10): the polling period; also lets the idle task feed the task watchdog
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
