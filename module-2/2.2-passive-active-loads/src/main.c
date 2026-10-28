#include <stdio.h>              // printf

#include "freertos/FreeRTOS.h"  // FreeRTOS types and settings; must come before task.h
#include "freertos/task.h"      // vTaskDelay
#include "driver/gpio.h"        // gpio_reset_pin, gpio_set_direction, gpio_set_level

// ESP-IDF calls app_main() once, from its main_task; returning would end that task, so the loop lives here.
void app_main(void) {
    // Arduino's pinMode(7, OUTPUT), in two steps:
    gpio_reset_pin(GPIO_NUM_7);                        // plain GPIO, old setup cleared (also enables the pull-up)
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);  // output driver on: the pin now drives 3.3 V or 0 V

    while (true) {                        // true is a keyword since C23 (ESP-IDF builds C with -std=gnu23)
        gpio_set_level(GPIO_NUM_7, 1);    // HIGH: the BC547 conducts, IN goes low, relay on
        printf("tick\n");                 // stdout is line-buffered: without \n nothing appears
        vTaskDelay(pdMS_TO_TICKS(1000));  // sleep 1000 ms = 100 ticks of 10 ms; other tasks run meanwhile
        gpio_set_level(GPIO_NUM_7, 0);    // LOW: the BC547 is off, the pull-up holds IN at 5 V, relay off
        vTaskDelay(pdMS_TO_TICKS(1000));  // off for 1 s too: without it the relay never releases
    }
}
