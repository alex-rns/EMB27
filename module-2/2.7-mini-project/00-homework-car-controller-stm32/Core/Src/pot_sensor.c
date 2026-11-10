/* pot_sensor: the potentiometer, read by the ADC */
#include "pot_sensor.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1; /* defined by CubeMX in main.c; ADC1 channel 1 = PA1 */

/* Written and read by the superloop only (no interrupt touches it), so no volatile */
static uint16_t s_last_raw = 0;

void pot_sensor_update(void)
{
	HAL_ADC_Start(&hadc1); /* start one conversion */
	if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) { /* wait for it, 1 ms at most */
		s_last_raw = (uint16_t)HAL_ADC_GetValue(&hadc1); /* 12 bits fit in uint16_t */
	}
}

uint16_t pot_sensor_get_raw(void)
{
	return s_last_raw;
}
