/* led: the indicator LED on PB8 */
#include "led.h"
#include "main.h"

void led_set(bool on)
{
	/* PB8 -> 220 ohm -> LED -> GND: HIGH lights it (CubeMX writes LOW before making PB8 an output, so it starts dark) */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
