#include "servo.h"

#define SERVO_CENTER_US 1500 // the middle of the travel on hobby servos

void app_main(void)
{
    servo_init();
    servo_set_pulse_us(SERVO_CENTER_US);
}
