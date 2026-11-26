#include "app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LOOP_MS 20 // one servo period, as in the lecture; app_run() takes about 7 ms of it (128 ADC readings)

void app_main(void)
{
    app_init();
    while (1) {
        app_run();
        vTaskDelay(pdMS_TO_TICKS(LOOP_MS)); // the task sleeps; the CPU is free meanwhile
    }
}
