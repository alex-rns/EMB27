/* load_driver: the loads (relay, buzzer and the turn signal LEDs) */
#include "load_driver.h"
#include "main.h" /* the pin names from CubeMX: RELAY_Pin, RELAY_GPIO_Port, ... */

/* on = true drives the pin HIGH: each load is switched by an NPN transistor or lit straight from the pin */

void load_relay_set(bool on)
{
	HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void load_buzzer_set(bool on)
{
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void load_led_left_set(bool on)
{
	HAL_GPIO_WritePin(LED_LEFT_GPIO_Port, LED_LEFT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void load_led_right_set(bool on)
{
	HAL_GPIO_WritePin(LED_RIGHT_GPIO_Port, LED_RIGHT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* A short beep. Blocking for now: the whole superloop waits; step 9 makes it non-blocking */
void load_buzzer_chirp(uint32_t ms)
{
	load_buzzer_set(true);
	HAL_Delay(ms);
	load_buzzer_set(false);
}
