#ifndef BUTTON_FSM_H
#define BUTTON_FSM_H
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_SHORT,
    BUTTON_DOUBLE,
    BUTTON_LONG,
    BUTTON_STUCK
} button_event_t;

typedef void (*button_event_cb_t)(button_event_t event, void *ctx);

typedef struct {
    bool raw_prev;
    bool stable_pressed;
    bool long_emitted;
    bool stuck_emitted;
    bool pending_single;
    bool second_press;
    int64_t raw_change_us;
    int64_t press_start_us;
    int64_t first_release_us;
} button_fsm_t;

// All timestamps in microseconds, monotonic, provided by caller.
void button_fsm_init(button_fsm_t *fsm, int64_t now_us);
void button_fsm_poll(button_fsm_t *fsm, bool raw_pressed, int64_t now_us,
                     button_event_cb_t emit, void *ctx);
#endif
