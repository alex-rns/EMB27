/* buzzer: the passive buzzer on PB1 (TIM3 CH4 PWM): beeps and the flasher's click */
#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include <stdbool.h>
#include <stdint.h>

void buzzer_init(void); /* call once after MX_TIM3_Init(): starts the buzzer timer, silent */

/* Non-blocking: these start a beep and return at once; buzzer_update() ends it */
void buzzer_beep(uint32_t ms);                 /* one beep of ms */
void buzzer_beeps(uint32_t ms, uint8_t count); /* count beeps of ms, with gaps of ms between them */
void buzzer_update(void);                      /* call on every superloop pass */

void buzzer_click(bool high); /* the flasher's tick-tock: the pin held HIGH or LOW; skipped during a beep */

#endif /* INC_BUZZER_H_ */
