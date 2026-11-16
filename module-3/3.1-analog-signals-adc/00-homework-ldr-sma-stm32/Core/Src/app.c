/* app: the night light: the LDR's moving average switches the LED with two thresholds (main.c calls only these two) */
#include "app.h"
#include <stdbool.h>
#include <stdint.h>
#include "hysteresis.h"
#include "ldr_sensor.h"
#include "led.h"
#include "main.h" /* HAL_GetTick() */
#include "sma.h"

#define LDR_SAMPLE_MS 50u  /* how often the LDR is read: room light changes slowly, 20 times a second is plenty */
#define LDR_AVG_SAMPLES 8u /* how many of the latest readings are averaged: 8 x 50 ms = the last 0.4 s */

/* Two thresholds (hysteresis) on the average. Measured on this breadboard: room light ~3290, a finger over the
   LDR ~1620. Between the two the LED keeps its state, so small changes of light don't make it flicker. */
#define LED_ON_BELOW  2000u /* darker than this: LED on (~1.6 V at PA1) */
#define LED_OFF_ABOVE 2600u /* lighter than this: LED off (~2.1 V at PA1) */
_Static_assert(LED_ON_BELOW < LED_OFF_ABOVE, "the on threshold must be below the off threshold");

/* Kept only to be watched from outside (the ST-Link reads RAM while the program runs). Nothing in the program reads
   them back, so without volatile the compiler could drop the writes. */
static volatile uint16_t s_ldr_raw; /* the latest reading */
static volatile uint16_t s_ldr_avg; /* the latest average */

static uint32_t s_last_ldr_ms;                  /* when ldr_slot() last read the LDR */
static uint16_t s_ldr_samples[LDR_AVG_SAMPLES]; /* the average's buffer: sma.c fills it */
static sma_t s_ldr_sma;                         /* the moving average of the LDR readings */
static hysteresis_t s_ldr_dark;                 /* dark or not, from the average with the two thresholds */

/* true once every period_ms, then *last_ms moves to now; the difference survives HAL_GetTick() wrapping after
   49.7 days */
static bool period_elapsed(uint32_t *last_ms, uint32_t now, uint32_t period_ms)
{
	if ((now - *last_ms) < period_ms) {
		return false; /* not yet */
	}
	*last_ms = now;
	return true;
}

/* What one good reading does: the new average decides the LED */
static void ldr_handle_reading(uint16_t raw)
{
	uint16_t avg = sma_add(&s_ldr_sma, raw);
	s_ldr_raw = raw;
	s_ldr_avg = avg;
	/* only a clear change of light switches the LED; between the thresholds it stays as it is */
	led_set(hysteresis_update(&s_ldr_dark, avg));
}

/* Every LDR_SAMPLE_MS: one reading */
static void ldr_slot(uint32_t now)
{
	if (!period_elapsed(&s_last_ldr_ms, now, LDR_SAMPLE_MS)) {
		return;
	}
	uint16_t raw;
	if (ldr_sensor_read(&raw)) { /* a failed conversion is skipped: the average keeps the older readings */
		ldr_handle_reading(raw);
	}
}

void app_init(void)
{
	sma_init(&s_ldr_sma, s_ldr_samples, LDR_AVG_SAMPLES);
	hysteresis_init(&s_ldr_dark, LED_ON_BELOW, LED_OFF_ABOVE, false); /* the LED starts dark, as CubeMX left PB8 */
}

void app_loop(void)
{
	ldr_slot(HAL_GetTick());
}
