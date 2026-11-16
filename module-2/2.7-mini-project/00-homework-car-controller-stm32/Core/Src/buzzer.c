/* buzzer: the passive buzzer on PB1 (TIM3 CH4 PWM): beeps and the flasher's click */
#include "buzzer.h"
#include "main.h" /* the HAL */

extern TIM_HandleTypeDef htim3; /* defined by CubeMX in main.c; TIM3 CH4 = the passive buzzer on PB1 */

/* The buzzer tone: TIM3 counts 0..1249 at 1 MHz = 800 Hz. CCR sets how long the pin stays HIGH each period:
   0 = silent, more = louder (up to half of the period, 625) */
#define BUZZER_CCR_TONE 156u /* about 1/8 of the period: a soft beep */

/* The buzzer's whole state in one struct: a series of beeps, its non-blocking timer */
typedef struct {
	bool active;       /* a series of beeps is playing */
	bool tone_on;      /* inside the series: a beep (true) or the gap after it (false) */
	uint8_t left;      /* beeps left in the series, the current one included */
	uint32_t start_ms; /* when the current beep or gap began */
	uint32_t len_ms;   /* one beep, and one gap, lasts this long */
} buzzer_t;

static buzzer_t s_buzzer; /* static: starts all zero, so no beep is playing */

void buzzer_init(void)
{
	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4) != HAL_OK) { /* CCR is 0 from CubeMX, so it starts silent */
		Error_Handler(); /* CubeMX's own reaction to a failed init: interrupts off, stop here */
	}
}

/* on = the tone, off = silence (the timer keeps running, only CCR changes) */
static void tone_set(bool on)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, on ? BUZZER_CCR_TONE : 0u);
}

/* The flasher "tick-tock": a passive buzzer clicks only when its level changes, so hold the pin steady.
   CCR above ARR keeps the PWM output HIGH for the whole period, CCR 0 keeps it LOW.
   While HIGH, at most 15 mA flows (3.3 V / 220 ohm): within the pin's limit */
void buzzer_click(bool high)
{
	if (s_buzzer.active) {
		return; /* don't cut a beep; the next click sets the level */
	}
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, high ? (__HAL_TIM_GET_AUTORELOAD(&htim3) + 1u) : 0u);
}

/* A series of beeps: switch the tone on, remember when, return at once; buzzer_update() does the rest */
void buzzer_beeps(uint32_t ms, uint8_t count)
{
	if (count == 0u) {
		return;
	}
	tone_set(true);
	s_buzzer.active = true;
	s_buzzer.tone_on = true;
	s_buzzer.left = count;
	s_buzzer.start_ms = HAL_GetTick();
	s_buzzer.len_ms = ms;
}

void buzzer_beep(uint32_t ms)
{
	buzzer_beeps(ms, 1u);
}

void buzzer_update(void)
{
	/* Its own HAL_GetTick(), not the loop's now: a beep started later in the same pass has start > now,
	   and now - start would wrap to a huge number and end the beep at once */
	uint32_t now = HAL_GetTick();

	if (s_buzzer.active && ((now - s_buzzer.start_ms) >= s_buzzer.len_ms)) {
		s_buzzer.start_ms = now;
		if (s_buzzer.tone_on) { /* a beep just ended: silence; a gap follows if beeps are left */
			tone_set(false);
			s_buzzer.tone_on = false;
			s_buzzer.left--;
			if (s_buzzer.left == 0u) {
				s_buzzer.active = false;
			}
		} else { /* the gap ended: the next beep */
			tone_set(true);
			s_buzzer.tone_on = true;
		}
	}
}
