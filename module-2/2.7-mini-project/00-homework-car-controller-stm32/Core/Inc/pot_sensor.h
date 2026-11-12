/* pot_sensor: the potentiometer, read by the ADC */
#ifndef INC_POT_SENSOR_H_
#define INC_POT_SENSOR_H_

#include <stdint.h>

#define POT_SAMPLE_MS 50u /* how often the superloop reads the pot: a knob moves slowly, 20 times a second is plenty */
#define POT_RAW_MAX   4095u /* the largest reading: a 12-bit ADC gives 0..4095 */

void pot_sensor_update(void);      /* one ADC conversion (about 4 us); the superloop calls it every POT_SAMPLE_MS */
uint16_t pot_sensor_get_raw(void); /* the last value, 0..4095 (12 bits) */

#endif /* INC_POT_SENSOR_H_ */
