#include <stdio.h>              // printf

#include "freertos/FreeRTOS.h"  // FreeRTOS types and settings; must come before task.h
#include "freertos/task.h"      // vTaskDelay
#include "driver/gpio.h"        // gpio_*: pin setup, set and read levels
#include "esp_timer.h"

// ESP-IDF calls app_main() once, from its main_task; returning would end that task, so the loop lives here.
void app_main(void) {
    // Arduino's pinMode(7, OUTPUT), in two steps:
    gpio_reset_pin(GPIO_NUM_7);                        // plain GPIO, old setup cleared (also enables the pull-up)
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);  // output driver on: the pin now drives 3.3 V or 0 V

    // Arduino's pinMode(9, INPUT_PULLUP), in three steps:
    gpio_reset_pin(GPIO_NUM_9);                        // plain GPIO, old setup cleared
    gpio_set_direction(GPIO_NUM_9, GPIO_MODE_INPUT);   // input only: the pin reads, never drives
    gpio_set_pull_mode(GPIO_NUM_9, GPIO_PULLUP_ONLY);  // open contact reads 1 (pull-up), closed reads 0 (COM = GND)

    while (true) {  // true is a keyword since C23 (ESP-IDF builds C with -std=gnu23)
        // ---- ON: command -> contact closes ----
        int64_t t0 = esp_timer_get_time();          // microseconds since boot, like Arduino's micros()
        gpio_set_level(GPIO_NUM_7, 1);              // HIGH: the BC547 conducts, IN goes low, relay on
        while (gpio_get_level(GPIO_NUM_9) == 1) {}  // busy-wait while the contact is still open (1 = open)
        int64_t t1 = esp_timer_get_time();          // first touch of the contact (bounce comes after it)
        printf("relay ON:  contact closed after %lld us\n", t1 - t0);  // int64_t is long long here: %lld
        vTaskDelay(pdMS_TO_TICKS(1000));            // stay on 1 s = 100 ticks of 10 ms; other tasks run meanwhile

        // ---- OFF: command -> contact opens ----
        t0 = esp_timer_get_time();                  // reuse t0/t1: declared once above, only assigned here
        gpio_set_level(GPIO_NUM_7, 0);              // LOW: the BC547 is off, the pull-up holds IN at 5 V, relay off
        while (gpio_get_level(GPIO_NUM_9) == 0) {}  // busy-wait while the contact is still closed (0 = closed)
        t1 = esp_timer_get_time();                  // the contact has just opened
        printf("relay OFF: contact opened after %lld us\n\n", t1 - t0);  // blank line between cycles
        vTaskDelay(pdMS_TO_TICKS(1000));            // stay off 1 s too: without it the relay never releases
    }
}
