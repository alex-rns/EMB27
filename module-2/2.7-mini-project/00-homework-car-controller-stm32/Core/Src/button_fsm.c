/* button_fsm: the buttons (debounce FSM) and the crash sensor input */
#include "button_fsm.h"
#include "main.h" /* the pin names from CubeMX: CRASH_Pin, ... */

/* The only place that knows the wiring: internal pull-up, the button to GND, so pressed = LOW.
   The FSM below works with "pressed or not" only */
static bool raw_pressed(const button_fsm_t *btn)
{
	return HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET;
}

/* No pin setup here: CubeMX already made the pin an input with a pull-up in MX_GPIO_Init() */
void button_fsm_init(button_fsm_t *btn, GPIO_TypeDef *port, uint16_t pin)
{
	btn->port = port; /* btn points to the caller's struct: these lines fill that struct, not a copy */
	btn->pin = pin;
	btn->state = BTN_STATE_IDLE;
	btn->state_enter_ms = 0;
	btn->long_fired = false;
}

/* Called every BTN_FSM_TICK_MS (10 ms) by the superloop; returns an event or BTN_EVENT_NONE */
button_event_t button_fsm_tick(button_fsm_t *btn, uint32_t now_ms)
{
	bool pressed = raw_pressed(btn);
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

/* Set by the interrupt, read by the superloop: volatile, so the compiler re-reads it every time */
static volatile bool s_crash_flag = false;

/* HAL calls this from EXTI9_5_IRQHandler(), inside the interrupt; it replaces HAL's empty __weak version.
   Only set the flag: no HAL_Delay here (SysTick can't run during this higher-priority interrupt, so it would hang) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CRASH_Pin) {
		s_crash_flag = true;
	}
}

bool crash_sensor_take(void)
{
	if (!s_crash_flag) {
		return false;
	}
	s_crash_flag = false; /* an interrupt between the read and this line merges two crashes into one: same reaction */
	return true;
}
