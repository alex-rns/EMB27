#pragma once

// A hobby servo on one LEDC channel: a pulse every 20 ms (50 Hz), its width sets the angle.
#define SERVO_MAX_DEG 180.0f // the travel: 0..180 degrees from the leftmost position

void servo_init(void);
void servo_set_angle(float angle_deg); // held inside 0..SERVO_MAX_DEG
