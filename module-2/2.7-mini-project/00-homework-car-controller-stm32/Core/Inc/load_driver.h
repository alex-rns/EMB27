/* load_driver: the loads (relay, buzzer and the turn signal LEDs) */
#ifndef INC_LOAD_DRIVER_H_
#define INC_LOAD_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

void load_relay_set(bool on);
void load_buzzer_set(bool on);
void load_led_left_set(bool on);
void load_led_right_set(bool on);
void load_buzzer_chirp(uint32_t ms);

#endif /* INC_LOAD_DRIVER_H_ */
