#include <stdio.h>              // printf

#include "freertos/FreeRTOS.h"  // FreeRTOS types and settings; must come before task.h
#include "freertos/task.h"      // vTaskDelay
#include "driver/gpio.h"        // gpio_*: pin setup, set and read levels, pin interrupts
#include "esp_timer.h"          // esp_timer_get_time: microseconds since boot

// Shared between the interrupt and app_main, so volatile: the compiler must re-read them every time.
static volatile int64_t edge_time = 0;   // when the contact first changed after the command, in us
static volatile bool waiting = false;    // true from the command until the first edge

// Arduino's attachInterrupt handler. Runs on every edge of GPIO 9, bounce included;
// only the first edge after the command is kept, the bounce that follows is ignored.
static void IRAM_ATTR contact_isr(void *arg) {  // IRAM_ATTR: code in RAM, as in Arduino; arg: unused
    (void)arg;                                   // silences -Wextra's unused-parameter warning
    if (waiting) {
        edge_time = esp_timer_get_time();
        waiting = false;                         // later edges (bounce) see false and do nothing
    }
}

// ESP-IDF calls app_main() once, from its main_task; returning would end that task, so the loop lives here.
void app_main(void) {
    // Arduino's pinMode(7, OUTPUT), in two steps:
    gpio_reset_pin(GPIO_NUM_7);                        // plain GPIO, old setup cleared (also enables the pull-up)
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);  // output driver on: the pin now drives 3.3 V or 0 V

    // Arduino's pinMode(9, INPUT_PULLUP), in three steps:
    gpio_reset_pin(GPIO_NUM_9);                        // plain GPIO, old setup cleared
    gpio_set_direction(GPIO_NUM_9, GPIO_MODE_INPUT);   // input only: the pin reads, never drives
    gpio_set_pull_mode(GPIO_NUM_9, GPIO_PULLUP_ONLY);  // open contact reads 1 (pull-up), closed reads 0 (COM = GND)

    // Arduino's attachInterrupt(9, contact_isr, CHANGE), in three steps:
    gpio_set_intr_type(GPIO_NUM_9, GPIO_INTR_ANYEDGE);     // CHANGE: both 1 -> 0 and 0 -> 1
    gpio_install_isr_service(0);                           // once per program: the per-pin interrupt dispatcher
    gpio_isr_handler_add(GPIO_NUM_9, contact_isr, NULL);   // NULL: no argument for the handler

    int64_t sum_on = 0, sum_off = 0;  // all times so far, in us
    int count = 0;                    // cycles so far

    while (true) {  // true is a keyword since C23 (ESP-IDF builds C with -std=gnu23)
        // ---- ON: command -> contact closes ----
        edge_time = 0;                              // 0 = no edge yet: the time comes out negative
        waiting = true;                             // arm the interrupt before the command
        int64_t t0 = esp_timer_get_time();          // microseconds since boot, like Arduino's micros()
        gpio_set_level(GPIO_NUM_7, 1);              // HIGH: the BC547 conducts, IN goes low, relay on
        vTaskDelay(pdMS_TO_TICKS(1000));            // stay on 1 s; the interrupt records the edge meanwhile
        int64_t on_time = edge_time - t0;           // edge_time is written once, long before this read

        // ---- OFF: command -> contact opens ----
        edge_time = 0;
        waiting = true;
        t0 = esp_timer_get_time();
        gpio_set_level(GPIO_NUM_7, 0);              // LOW: the BC547 is off, the pull-up holds IN at 5 V, relay off
        vTaskDelay(pdMS_TO_TICKS(1000));            // stay off 1 s too: without it the relay never releases
        int64_t off_time = edge_time - t0;

        // ---- print, and average only the good cycles ----
        if (on_time < 0 || off_time < 0) {         // no edge within 1 s: the relay has no power, say
            printf("no contact change, cycle skipped\n\n");
            continue;                               // straight to the next cycle
        }
        printf("relay ON:  contact closed after %lld us\n", on_time);  // int64_t is long long here: %lld
        printf("relay OFF: contact opened after %lld us\n", off_time);
        sum_on += on_time;
        sum_off += off_time;
        count++;
        printf("average of %d: ON %lld us, OFF %lld us\n\n", count, sum_on / count, sum_off / count);
    }
}
