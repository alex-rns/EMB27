/* led: the indicator LED on PB8 */
#ifndef INC_LED_H_
#define INC_LED_H_

#include <stdbool.h>

void led_set(bool on); /* true: lit, false: dark (which pin level that takes is the driver's business) */

#endif /* INC_LED_H_ */
