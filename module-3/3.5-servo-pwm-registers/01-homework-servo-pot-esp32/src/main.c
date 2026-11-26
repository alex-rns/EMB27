#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pot.h"
#include "servo.h"

#define LOOP_MS 20 // one servo period, as in the lecture

void app_main(void)
{
    servo_init();
    pot_init();

    for (;;) {
        // Point 1: 1:1, both counted from their leftmost position.
        const float pot_deg = pot_read_deg();

        // Point 2: the pot turns about 300 deg, the servo 180; only the shared 0..180 is used, past it the servo
        // stays at 180.
        const float servo_deg = pot_deg < SERVO_MAX_DEG ? pot_deg : SERVO_MAX_DEG;
        servo_set_angle(servo_deg);

        vTaskDelay(pdMS_TO_TICKS(LOOP_MS)); // the task sleeps; the CPU is free meanwhile
    }
}
