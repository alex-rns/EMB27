/* crash_sensor: the crash input (PA8, an EXTI interrupt on the falling edge, set up in CubeMX) */
#include "crash_sensor.h"
#include "main.h" /* the pin names from CubeMX: CRASH_Pin */

/* Set by the interrupt, read by the superloop: volatile, so the compiler re-reads it every time */
static volatile bool s_crash_flag = false;

/* HAL calls this from EXTI9_5_IRQHandler(), inside the interrupt; it replaces HAL's empty __weak version.
   Only set the flag: no HAL_Delay here (SysTick can't run during this higher-priority interrupt, so it would hang) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CRASH_Pin) {
		s_crash_flag = true;
	}
}

bool crash_sensor_take(void)
{
	if (!s_crash_flag) {
		return false;
	}
	s_crash_flag = false; /* an interrupt between the read and this line merges two crashes into one: same reaction */
	return true;
}
