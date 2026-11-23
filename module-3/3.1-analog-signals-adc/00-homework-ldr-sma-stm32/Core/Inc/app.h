/* app: the night light: the LDR's moving average switches the LED with two thresholds (main.c calls only these two) */
#ifndef INC_APP_H_
#define INC_APP_H_

void app_init(void); /* once, after CubeMX's inits */
void app_loop(void); /* on every superloop pass; blocks at most one ADC conversion (3.84 us, 2 ms if the ADC hangs) */

#endif /* INC_APP_H_ */
