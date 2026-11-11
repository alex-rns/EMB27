#include "button_fsm.h"
#include <stddef.h>

#define DEBOUNCE_US       30000LL
#define DOUBLE_CLICK_US  350000LL
#define LONG_PRESS_US   2000000LL
#define STUCK_US        7000000LL

// Calls the callback registered for this event, if there is one
static void publish(const button_fsm_t *s, button_event_t ev) {
    button_event_cb_t cb = s->handlers[ev];
    if (cb != NULL) cb(ev, s->handler_ctx[ev]);
}

void button_fsm_init(button_fsm_t *fsm, int64_t now_us) {
    *fsm = (button_fsm_t){0};   // also sets every handler to NULL and the state to IDLE
    fsm->raw_change_us = now_us;
}

void button_fsm_on(button_fsm_t *fsm, button_event_t event, button_event_cb_t cb, void *ctx) {
    if (event >= BUTTON_EVENT_COUNT) return;   // a bad index would write past the end of the arrays
    fsm->handlers[event] = cb;
    fsm->handler_ctx[event] = ctx;
}

void button_fsm_poll(button_fsm_t *s, bool raw, int64_t now) {
    // Debouncer: remember when the raw level last changed
    if (raw != s->raw_prev) {
        s->raw_prev = raw;
        s->raw_change_us = now;
    }

    // Debouncer: a level held for DEBOUNCE_US becomes the stable one
    bool pressed_edge = false;    // the button has just become stably pressed
    bool released_edge = false;   // the button has just become stably released
    if (raw != s->stable_pressed && (now - s->raw_change_us >= DEBOUNCE_US)) {
        s->stable_pressed = raw;
        pressed_edge = raw;
        released_edge = !raw;
    }

    // Gestures: one state at a time, fed only with the stable edges above
    switch (s->state) {
    case IDLE:
        if (pressed_edge) {
            s->press_start_us = now;
            s->state = HELD;
        }
        break;

    case HELD:
        if (released_edge) {
            s->first_release_us = now;   // the double-click window opens
            s->state = WAIT_SECOND;
        } else if (now - s->press_start_us >= LONG_PRESS_US) {
            s->state = LONG_HELD;
            publish(s, BUTTON_LONG);
        }
        break;

    case WAIT_SECOND:
        if (pressed_edge) {
            s->press_start_us = now;
            s->state = SECOND_HELD;
        } else if (now - s->first_release_us >= DOUBLE_CLICK_US) {
            s->state = IDLE;
            publish(s, BUTTON_SHORT);   // no second press in time
        }
        break;

    case SECOND_HELD:
        if (released_edge) {
            s->state = IDLE;
            publish(s, BUTTON_DOUBLE);
        } else if (now - s->press_start_us >= LONG_PRESS_US) {
            s->state = LONG_HELD;
            publish(s, BUTTON_LONG);    // the first short press is dropped, as in the lecture code
        }
        break;

    case LONG_HELD:
        if (released_edge) {
            s->state = IDLE;                    // no SHORT after a LONG
        } else if (now - s->press_start_us >= STUCK_US) {
            s->state = STUCK_HELD;
            publish(s, BUTTON_STUCK);
        }
        break;

    case STUCK_HELD:
        if (released_edge) {
            s->state = IDLE;                    // back to normal only once the button is released
        }
        break;
    }
}
