/* relay: the relay on PB0 (through the BC547B stage): the steady lock level and timed pulses */
#include "relay.h"
#include "main.h" /* the pin names from CubeMX: RELAY_Pin, RELAY_GPIO_Port */

static bool s_relay_level = false; /* what relay_set() asked for: the lock state */
static bool s_pulse_active = false;
static uint32_t s_pulse_start_ms;
static uint32_t s_pulse_len_ms;

/* on = true drives the pin HIGH: the relay through an NPN transistor */
static void relay_write(bool on)
{
	HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void relay_set(bool on)
{
	s_relay_level = on;
	if (!s_pulse_active) { /* during a pulse the relay stays on; relay_update() applies the level after it */
		relay_write(on);
	}
}

void relay_pulse(uint32_t ms)
{
	relay_write(true);
	s_pulse_active = true;
	s_pulse_start_ms = HAL_GetTick();
	s_pulse_len_ms = ms;
}

void relay_update(void)
{
	uint32_t now = HAL_GetTick(); /* its own tick, for the same reason as in buzzer_update() */

	if (s_pulse_active && ((now - s_pulse_start_ms) >= s_pulse_len_ms)) {
		relay_write(s_relay_level); /* back to the lock state */
		s_pulse_active = false;
	}
}
