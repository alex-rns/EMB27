/* button_fsm: the debounce FSM of one button. Pure logic: it never reads a pin. The caller passes "pressed or not",
   so the FSM doesn't know the wiring (port, pin, pull-up) and works for any button on any board */
#ifndef INC_BUTTON_FSM_H_
#define INC_BUTTON_FSM_H_
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	BTN_STATE_IDLE,
	BTN_STATE_PRESSED,
	BTN_STATE_HELD,
	BTN_STATE_RELEASE,
} button_state_t;

typedef enum {
	BTN_EVENT_NONE,
	BTN_EVENT_SHORT_PRESS,
	BTN_EVENT_LONG_PRESS,
} button_event_t;

/* One button's memory between two button_fsm_tick() calls */
typedef struct {
	button_state_t state;    /* current FSM state */
	uint32_t state_enter_ms; /* HAL_GetTick() when the current state began */
	bool long_fired;         /* LONG already reported for this press: report it once */
} button_fsm_t;

/* Times in ms; the u suffix makes them unsigned, like HAL_GetTick() */
#define BTN_DEBOUNCE_MS   50u   /* the level must stay stable this long, on press and on release */
#define BTN_LONG_PRESS_MS 2000u /* held this long = LONG */
/* How often the superloop calls button_fsm_tick(): about 5 checks inside the 50 ms debounce window. At 20 ms only
   2-3 checks, and a 5-10 ms bounce pulse slips between them more easily; 5 ms works too but gains nothing a hand feels */
#define BTN_FSM_TICK_MS   10u

/* Start one button in IDLE */
void button_fsm_init(button_fsm_t *btn);

/* Advance one button's FSM with the level read just now; returns an event once per press, otherwise BTN_EVENT_NONE */
button_event_t button_fsm_tick(button_fsm_t *btn, bool pressed, uint32_t now_ms);

#endif /* INC_BUTTON_FSM_H_ */
