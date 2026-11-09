/* button_fsm: the buttons (debounce FSM) and the crash sensor input */
#ifndef INC_BUTTON_FSM_H_
#define INC_BUTTON_FSM_H_
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

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
	GPIO_TypeDef *port;      /* GPIO port, e.g. GPIOB */
	uint16_t pin;            /* pin mask, e.g. GPIO_PIN_12 */
	button_state_t state;    /* current FSM state */
	uint32_t state_enter_ms; /* HAL_GetTick() when the current state began */
	bool long_fired;         /* LONG already reported for this press: report it once */
} button_fsm_t;

/* Times in ms; the u suffix makes them unsigned, like HAL_GetTick() */
#define BTN_DEBOUNCE_MS   50u   /* the level must stay stable this long, on press and on release */
#define BTN_LONG_PRESS_MS 2000u /* held this long = LONG */
#define BTN_FSM_TICK_MS   10u   /* how often the superloop calls button_fsm_tick() */

/* Set up one button: remember its pin, start in IDLE */
void button_fsm_init(button_fsm_t *btn, GPIO_TypeDef *port, uint16_t pin);

/* Advance one button's FSM; returns an event once per press, otherwise BTN_EVENT_NONE */
button_event_t button_fsm_tick(button_fsm_t *btn, uint32_t now_ms);

#endif /* INC_BUTTON_FSM_H_ */
