/* load_driver: the loads (relay, buzzer and the turn signal LEDs) */
#ifndef INC_LOAD_DRIVER_H_
#define INC_LOAD_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

void load_driver_init(void); /* call once after MX_TIM3_Init(): starts the buzzer timer, silent */
void load_relay_set(bool on);   /* the steady relay level (the lock state) */
void load_led_left_set(bool on);
void load_led_right_set(bool on);
void load_led_valet_set(bool on); /* the onboard LED (PC13), lit on LOW: the driver flips the level */
void load_buzzer_tick(bool high); /* the flasher click: the buzzer pin held HIGH or LOW; skipped during a chirp */

/* Non-blocking: these start and return at once; load_driver_update() ends them */
void load_buzzer_chirp(uint32_t ms); /* a beep of ms */
void load_buzzer_chirps(uint32_t ms, uint8_t count); /* count beeps of ms, with gaps of ms between them */
void load_relay_pulse(uint32_t ms);  /* the relay on for ms, then back to the load_relay_set() level */
void load_driver_update(void);       /* call on every superloop pass */

#endif /* INC_LOAD_DRIVER_H_ */
