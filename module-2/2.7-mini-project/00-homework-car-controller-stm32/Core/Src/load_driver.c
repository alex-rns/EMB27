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

/* The non-blocking timers: when a beep or a relay pulse started and how long it lasts */
static bool s_chirp_active = false;
static uint32_t s_chirp_start_ms;
static uint32_t s_chirp_len_ms;

static bool s_relay_level = false; /* what load_relay_set() asked for: the lock state */
static bool s_pulse_active = false;
static uint32_t s_pulse_start_ms;
static uint32_t s_pulse_len_ms;

/* on = true drives the pin HIGH: the relay through an NPN transistor, the LEDs straight from the pin */

static void relay_write(bool on)
{
	HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void load_relay_set(bool on)
{
	s_relay_level = on;
	if (!s_pulse_active) { /* during a pulse the relay stays on; load_driver_update() applies the level after it */
		relay_write(on);
	}
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

/* A beep: switch the tone on, remember when, return at once; load_driver_update() switches it off */
void load_buzzer_chirp(uint32_t ms)
{
	load_buzzer_set(true);
	s_chirp_active = true;
	s_chirp_start_ms = HAL_GetTick();
	s_chirp_len_ms = ms;
}

void load_relay_pulse(uint32_t ms)
{
	relay_write(true);
	s_pulse_active = true;
	s_pulse_start_ms = HAL_GetTick();
	s_pulse_len_ms = ms;
}

void load_driver_update(void)
{
	/* Its own HAL_GetTick(), not the loop's now: a chirp started later in the same pass has start > now,
	   and now - start would wrap to a huge number and end the beep at once */
	uint32_t now = HAL_GetTick();

	if (s_chirp_active && now - s_chirp_start_ms >= s_chirp_len_ms) {
		load_buzzer_set(false);
		s_chirp_active = false;
	}
	if (s_pulse_active && now - s_pulse_start_ms >= s_pulse_len_ms) {
		relay_write(s_relay_level); /* back to the lock state */
		s_pulse_active = false;
	}
}
