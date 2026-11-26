#pragma once

#include <stdint.h>

// A hobby servo on one LEDC channel: a pulse every 20 ms (50 Hz), its width sets the angle.
#define SERVO_MAX_DEG 180 // the travel: 0..180 degrees from the leftmost position

void servo_init(void);
void servo_set_angle(uint32_t angle_deg); // a larger angle than SERVO_MAX_DEG is held at SERVO_MAX_DEG
