#ifndef BUTTON_FSM_H
#define BUTTON_FSM_H
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_SHORT,
    BUTTON_DOUBLE,
    BUTTON_LONG,
    BUTTON_STUCK,
    BUTTON_EVENT_COUNT,   // not an event: the number of events above, for array sizes
} button_event_t;

typedef enum {
    IDLE,
    HELD,
    WAIT_SECOND,
    SECOND_HELD,
    LONG_HELD,
    STUCK_HELD,
} button_state_t;

typedef void (*button_event_cb_t)(button_event_t event, void *ctx);

typedef struct {
    bool raw_prev;
    bool stable_pressed;
    int64_t raw_change_us;
    int64_t press_start_us;
    int64_t first_release_us;
    button_state_t state;
    button_event_cb_t handlers[BUTTON_EVENT_COUNT];   // one callback per event, indexed by the event; NULL = none
    void *handler_ctx[BUTTON_EVENT_COUNT];            // handed back to that event's callback
} button_fsm_t;

// All timestamps in microseconds, monotonic, provided by caller.
void button_fsm_init(button_fsm_t *fsm, int64_t now_us);
// Sets the callback for one event (after init: init clears them all). A second call for the same event
// replaces it; cb = NULL removes it. Events without a callback are ignored.
void button_fsm_on(button_fsm_t *fsm, button_event_t event, button_event_cb_t cb, void *ctx);
void button_fsm_poll(button_fsm_t *fsm, bool raw_pressed, int64_t now_us);
#endif
