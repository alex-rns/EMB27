/* led: the turn signal LEDs (PB8 left, PB9 right) and the onboard LED (PC13, the valet mode) */
#ifndef INC_LED_H_
#define INC_LED_H_

#include <stdbool.h>

void led_left_set(bool on);
void led_right_set(bool on);
void led_valet_set(bool on); /* the onboard LED (PC13), lit on LOW: the driver flips the level */

#endif /* INC_LED_H_ */
