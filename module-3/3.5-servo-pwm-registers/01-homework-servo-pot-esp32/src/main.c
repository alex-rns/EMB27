#include "servo.h"

void app_main(void)
{
    servo_init();
    servo_set_angle(SERVO_MAX_DEG / 2); // the middle of the travel
}
