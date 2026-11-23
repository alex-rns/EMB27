/* ldr_sensor: the LDR divider on PA1 (ADC1 channel 1), read by one ADC conversion at a time */
#include "ldr_sensor.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1; /* defined by CubeMX in main.c (single conversion, software start, 84-cycle sampling) */

/* The wait limit for one conversion, in HAL_GetTick() ticks = ms. HAL gives up only when MORE than this many ticks
   have passed (> Timeout), and the next tick may come right after the start, so the real limit is 1 to 2 ms.
   A conversion takes 84 + 12 ADC clocks at 25 MHz (PCLK2 100 MHz / 4) = 3.84 us, so a working ADC never reaches
   it: it only keeps the superloop from hanging if the conversion never ends (HAL_MAX_DELAY would wait forever). */
#define LDR_ADC_TIMEOUT_MS 1u

bool ldr_sensor_read(uint16_t *raw)
{
	/* start one conversion, then wait for it; && stops at the first failure, so no wait if the start failed */
	if ((HAL_ADC_Start(&hadc1) == HAL_OK) && (HAL_ADC_PollForConversion(&hadc1, LDR_ADC_TIMEOUT_MS) == HAL_OK)) {
		*raw = (uint16_t)HAL_ADC_GetValue(&hadc1); /* 12 bits fit in uint16_t */
		return true;
	}
	return false; /* *raw keeps its old value */
}
