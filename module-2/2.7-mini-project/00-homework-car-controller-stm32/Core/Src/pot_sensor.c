/* pot_sensor: the potentiometer, read by the ADC */
#include "pot_sensor.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1; /* defined by CubeMX in main.c; ADC1 channel 1 = PA1 */

/* The wait limit for one conversion, in HAL_GetTick() ticks = ms (HAL timeouts count ms, not seconds).
   HAL gives up only when MORE than this many ticks have passed (> Timeout), and the next tick may come right after
   the start, so the real limit is 1 to 2 ms. A conversion takes 84 + 12 ADC clocks at 25 MHz (PCLK2 100 MHz / 4)
   = 3.84 us, so a working ADC never reaches it: it only keeps the superloop from hanging if the conversion never
   ends (HAL_MAX_DELAY would wait forever) */
#define POT_ADC_TIMEOUT_MS 1u

/* Written and read by the superloop only (no interrupt touches it), so no volatile */
static uint16_t s_last_raw = 0;

void pot_sensor_update(void)
{
	/* start one conversion, then wait for it; && stops at the first failure, so no wait if the start failed */
	if (HAL_ADC_Start(&hadc1) == HAL_OK && HAL_ADC_PollForConversion(&hadc1, POT_ADC_TIMEOUT_MS) == HAL_OK) {
		s_last_raw = (uint16_t)HAL_ADC_GetValue(&hadc1); /* 12 bits fit in uint16_t */
	}
}

uint16_t pot_sensor_get_raw(void)
{
	return s_last_raw;
}
