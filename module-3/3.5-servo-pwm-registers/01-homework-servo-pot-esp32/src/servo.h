#pragma once

#include <stdint.h>

// A hobby servo on one LEDC channel: a pulse every 20 ms (50 Hz), its width sets the angle.
void servo_init(void);
void servo_set_pulse_us(uint32_t pulse_us); // the pulse width in microseconds, applied from the next period
