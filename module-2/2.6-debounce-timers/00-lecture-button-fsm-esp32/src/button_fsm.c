#include "button_fsm.h"
#include <stddef.h>

#define DEBOUNCE_US       30000LL
#define DOUBLE_CLICK_US  350000LL
#define LONG_PRESS_US   2000000LL
#define STUCK_US        7000000LL

static void publish(button_event_cb_t cb, button_event_t ev, void *ctx) {
    if (cb != NULL) cb(ev, ctx);
}

void button_fsm_init(button_fsm_t *fsm, int64_t now_us) {
    *fsm = (button_fsm_t){0};
    fsm->raw_change_us = now_us;
}

void button_fsm_poll(button_fsm_t *s, bool raw, int64_t now,
                     button_event_cb_t emit, void *ctx) {
    if (raw != s->raw_prev) {
        s->raw_prev = raw;
        s->raw_change_us = now;
    }

    // Defer single-click when double-click window closes.
    if (s->pending_single && !s->second_press &&
        (now - s->first_release_us >= DOUBLE_CLICK_US)) {
        s->pending_single = false;
        publish(emit, BUTTON_SHORT, ctx);
    }

    if (raw != s->stable_pressed && (now - s->raw_change_us >= DEBOUNCE_US)) {
        s->stable_pressed = raw;
        if (raw) {
            s->press_start_us = now;
            s->long_emitted = false;
            s->stuck_emitted = false;
            s->second_press = s->pending_single &&
                (now - s->first_release_us < DOUBLE_CLICK_US);
        } else {
            if (!s->long_emitted && !s->stuck_emitted) {
                if (s->second_press) {
                    s->pending_single = false;
                    publish(emit, BUTTON_DOUBLE, ctx);
                } else {
                    s->pending_single = true;
                    s->first_release_us = now;
                }
            }
            s->second_press = false;
        }
    }

    if (s->stable_pressed) {
        int64_t held = now - s->press_start_us;
        if (!s->long_emitted && held >= LONG_PRESS_US) {
            s->long_emitted = true;
            s->pending_single = false;
            publish(emit, BUTTON_LONG, ctx);
        }
        if (!s->stuck_emitted && held >= STUCK_US) {
            s->stuck_emitted = true;
            s->pending_single = false;
            publish(emit, BUTTON_STUCK, ctx);
        }
    }
}
