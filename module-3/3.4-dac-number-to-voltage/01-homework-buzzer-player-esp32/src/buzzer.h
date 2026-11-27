#pragma once

#include <stdint.h>

// A passive buzzer on one LEDC channel: the PWM's frequency is the tone, 50 % duty.
void buzzer_init(void);
void buzzer_tone(uint32_t freq_hz); // start or change a tone
void buzzer_off(void);              // silence: duty 0, the pin stays low
