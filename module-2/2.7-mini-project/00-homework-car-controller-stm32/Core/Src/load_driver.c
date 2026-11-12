/* load_driver: the loads (relay, buzzer and the turn signal LEDs) */
#include "load_driver.h"
#include "main.h" /* the pin names from CubeMX: RELAY_Pin, RELAY_GPIO_Port, ... */

extern TIM_HandleTypeDef htim3; /* defined by CubeMX in main.c; TIM3 CH4 = the passive buzzer on PB1 */

/* The buzzer tone: TIM3 counts 0..1249 at 1 MHz = 800 Hz. CCR sets how long the pin stays HIGH each period:
   0 = silent, more = louder (up to half of the period, 625) */
#define BUZZER_CCR_TONE 156u /* about 1/8 of the period: a soft beep */

/* The non-blocking timers: when a beep or a relay pulse started and how long it lasts */
static bool s_chirp_active = false;
static bool s_chirp_tone_on = false; /* inside a series: a beep (true) or the gap after it (false) */
static uint8_t s_chirp_left = 0;     /* beeps left in the series, the current one included */
static uint32_t s_chirp_start_ms;    /* when the current beep or gap began */
static uint32_t s_chirp_len_ms;      /* one beep, and one gap, lasts this long */

static bool s_relay_level = false; /* what load_relay_set() asked for: the lock state */
static bool s_pulse_active = false;
static uint32_t s_pulse_start_ms;
static uint32_t s_pulse_len_ms;

void load_driver_init(void)
{
	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4) != HAL_OK) { /* CCR is 0 from CubeMX, so it starts silent */
		Error_Handler(); /* CubeMX's own reaction to a failed init: interrupts off, stop here */
	}
}

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

/* on = the tone, off = silence (the timer keeps running, only CCR changes); static: only the chirps use it */
static void load_buzzer_set(bool on)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, on ? BUZZER_CCR_TONE : 0u);
}

/* The flasher "tick-tock": a passive buzzer clicks only when its level changes, so hold the pin steady.
   CCR above ARR keeps the PWM output HIGH for the whole period, CCR 0 keeps it LOW.
   While HIGH, at most 15 mA flows (3.3 V / 220 ohm): within the pin's limit */
void load_buzzer_tick(bool high)
{
	if (s_chirp_active) {
		return; /* don't cut a beep; the next tick sets the level */
	}
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, high ? (__HAL_TIM_GET_AUTORELOAD(&htim3) + 1u) : 0u);
}

void load_led_left_set(bool on)
{
	HAL_GPIO_WritePin(LED_LEFT_GPIO_Port, LED_LEFT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void load_led_right_set(bool on)
{
	HAL_GPIO_WritePin(LED_RIGHT_GPIO_Port, LED_RIGHT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* The Black Pill's onboard LED (PC13) shows the valet mode; its anode is on 3V3, so it is lit on LOW */
void load_led_valet_set(bool on)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* A series of beeps: switch the tone on, remember when, return at once; load_driver_update() does the rest */
void load_buzzer_chirps(uint32_t ms, uint8_t count)
{
	if (count == 0u) {
		return;
	}
	load_buzzer_set(true);
	s_chirp_active = true;
	s_chirp_tone_on = true;
	s_chirp_left = count;
	s_chirp_start_ms = HAL_GetTick();
	s_chirp_len_ms = ms;
}

void load_buzzer_chirp(uint32_t ms)
{
	load_buzzer_chirps(ms, 1u);
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

	if (s_chirp_active && ((now - s_chirp_start_ms) >= s_chirp_len_ms)) {
		s_chirp_start_ms = now;
		if (s_chirp_tone_on) { /* a beep just ended: silence; a gap follows if beeps are left */
			load_buzzer_set(false);
			s_chirp_tone_on = false;
			s_chirp_left--;
			if (s_chirp_left == 0u) {
				s_chirp_active = false;
			}
		} else { /* the gap ended: the next beep */
			load_buzzer_set(true);
			s_chirp_tone_on = true;
		}
	}
	if (s_pulse_active && ((now - s_pulse_start_ms) >= s_pulse_len_ms)) {
		relay_write(s_relay_level); /* back to the lock state */
		s_pulse_active = false;
	}
}
