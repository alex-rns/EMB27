#include <stdio.h>              // printf

#include "freertos/FreeRTOS.h"  // FreeRTOS types and settings; must come before task.h
#include "freertos/task.h"      // vTaskDelay

// ESP-IDF calls app_main() once, from its main_task; returning would end that task, so the loop lives here.
void app_main(void) {
    while (true) {                        // true is a keyword since C23 (ESP-IDF builds C with -std=gnu23)
        printf("tick\n");                 // stdout is line-buffered: without \n nothing appears
        vTaskDelay(pdMS_TO_TICKS(1000));  // sleep 1000 ms = 100 ticks of 10 ms; other tasks run meanwhile
    }
}
