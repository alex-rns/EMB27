/* led: the turn signal LEDs (PB8 left, PB9 right) and the onboard LED (PC13, the valet mode) */
#include "led.h"
#include "main.h" /* the pin names from CubeMX: LED_LEFT_Pin, LED_LEFT_GPIO_Port, ... */

/* The turn LEDs hang straight on the pins: on = HIGH */
void led_left_set(bool on)
{
	HAL_GPIO_WritePin(LED_LEFT_GPIO_Port, LED_LEFT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void led_right_set(bool on)
{
	HAL_GPIO_WritePin(LED_RIGHT_GPIO_Port, LED_RIGHT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* The Black Pill's onboard LED (PC13) shows the valet mode; its anode is on 3V3, so it is lit on LOW */
void led_valet_set(bool on)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
