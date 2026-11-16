/* relay: the relay on PB0 (through the BC547B stage): the steady lock level and timed pulses */
#include "relay.h"
#include "main.h" /* the pin names from CubeMX: RELAY_Pin, RELAY_GPIO_Port */

/* The relay's whole state in one struct: the lock level and a pulse's non-blocking timer */
typedef struct {
	bool level;              /* what relay_set() asked for: the lock state */
	bool pulse_active;       /* a pulse holds the relay on right now */
	uint32_t pulse_start_ms; /* when the pulse began */
	uint32_t pulse_len_ms;   /* how long it lasts */
} relay_t;

static relay_t s_relay; /* static: starts all zero, so off and no pulse */

/* on = true drives the pin HIGH: the relay through an NPN transistor */
static void relay_write(bool on)
{
	HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void relay_set(bool on)
{
	s_relay.level = on;
	if (!s_relay.pulse_active) { /* during a pulse the relay stays on; relay_update() applies the level after it */
		relay_write(on);
	}
}

void relay_pulse(uint32_t ms)
{
	relay_write(true);
	s_relay.pulse_active = true;
	s_relay.pulse_start_ms = HAL_GetTick();
	s_relay.pulse_len_ms = ms;
}

void relay_update(void)
{
	uint32_t now = HAL_GetTick(); /* its own tick, for the same reason as in buzzer_update() */

	if (s_relay.pulse_active && ((now - s_relay.pulse_start_ms) >= s_relay.pulse_len_ms)) {
		relay_write(s_relay.level); /* back to the lock state */
		s_relay.pulse_active = false;
	}
}
