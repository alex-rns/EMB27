/* button_fsm: the debounce FSM of one button (pure logic: no pins, no HAL) */
#include "button_fsm.h"

void button_fsm_init(button_fsm_t *btn)
{
	btn->state = BTN_STATE_IDLE; /* btn points to the caller's struct: these lines fill that struct, not a copy */
	btn->state_enter_ms = 0;
	btn->long_fired = false;
}

/* Called every BTN_FSM_TICK_MS (10 ms) by the superloop; pressed = the button's level read just now */
button_event_t button_fsm_tick(button_fsm_t *btn, bool pressed, uint32_t now_ms)
{
	button_event_t event = BTN_EVENT_NONE;

	switch (btn->state) { /* a switch on the enum: -Wswitch warns if a new state is left unhandled */
	case BTN_STATE_IDLE:
		if (pressed) {
			btn->state = BTN_STATE_PRESSED;
			btn->state_enter_ms = now_ms;
		}
		break;

	case BTN_STATE_PRESSED: /* pressed, but maybe only bounce: wait for 50 ms of a steady level */
		if (!pressed) {
			btn->state = BTN_STATE_IDLE; /* it dropped before 50 ms: that was bounce */
		} else if (now_ms - btn->state_enter_ms >= BTN_DEBOUNCE_MS) {
			btn->state = BTN_STATE_HELD;
			btn->state_enter_ms = now_ms; /* from now on it counts how long the button is held */
			btn->long_fired = false;
		}
		break;

	case BTN_STATE_HELD: {
		uint32_t held_ms = now_ms - btn->state_enter_ms;
		if (!pressed) {
			if (!btn->long_fired) {
				event = BTN_EVENT_SHORT_PRESS; /* released before the long threshold */
			}
			btn->state = BTN_STATE_RELEASE;
			btn->state_enter_ms = now_ms;
		} else if (!btn->long_fired && held_ms >= BTN_LONG_PRESS_MS) {
			event = BTN_EVENT_LONG_PRESS; /* reported while still held, and only once */
			btn->long_fired = true;
		}
		break;
	}

	case BTN_STATE_RELEASE: /* the mirror of PRESSED: wait for 50 ms of a steady release */
		if (pressed) {
			btn->state = BTN_STATE_HELD; /* bounce lifted it back: the press goes on */
		} else if (now_ms - btn->state_enter_ms >= BTN_DEBOUNCE_MS) {
			btn->state = BTN_STATE_IDLE;
		}
		break;
	}

	return event;
}
