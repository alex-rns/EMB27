/* load_driver: the loads (relay, buzzer and the turn signal LEDs) */
#include "load_driver.h"
#include "main.h" /* the pin names from CubeMX: RELAY_Pin, RELAY_GPIO_Port, ... */

extern TIM_HandleTypeDef htim3; /* defined by CubeMX in main.c; TIM3 CH4 = the passive buzzer on PB1 */

/* The buzzer tone: TIM3 counts 0..1249 at 1 MHz = 800 Hz. CCR sets how long the pin stays HIGH each period:
   0 = silent, more = louder (up to half of the period, 625) */
#define BUZZER_CCR_TONE 156u /* about 1/8 of the period: a soft beep */

void load_driver_init(void)
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4); /* CCR is 0 from CubeMX, so it starts silent */
}

/* on = true drives the pin HIGH: the relay through an NPN transistor, the LEDs straight from the pin */

void load_relay_set(bool on)
{
	HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* on = the tone, off = silence (the timer keeps running, only CCR changes) */
void load_buzzer_set(bool on)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, on ? BUZZER_CCR_TONE : 0u);
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
