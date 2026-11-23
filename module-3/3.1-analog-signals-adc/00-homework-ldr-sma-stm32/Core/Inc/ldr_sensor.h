/* ldr_sensor: the LDR divider on PA1 (ADC1 channel 1), read by one ADC conversion at a time */
#ifndef INC_LDR_SENSOR_H_
#define INC_LDR_SENSOR_H_

#include <stdbool.h>
#include <stdint.h>

bool ldr_sensor_read(uint16_t *raw); /* one conversion (blocks ~4 us): true and *raw = 0..4095, or false if it failed */

#endif /* INC_LDR_SENSOR_H_ */
