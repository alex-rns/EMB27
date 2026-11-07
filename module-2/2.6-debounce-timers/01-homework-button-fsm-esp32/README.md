# 01 — Homework (extra): the button state machine as a switch-case, with a callback per event

The teacher's extra task for lesson 2.6, on the lecture's button state machine (step
[00](../00-lecture-button-fsm-esp32/)). ESP-IDF in C. The folder starts from the lecture code unchanged (its first
commit, on `main` since pull request #25), so the homework's pull request and each later commit show only what this
homework changes.

## Assignment

"Extra homework 2_6" (the teacher's message in the group chat, translated in full):

> Rewrite the state machine as a switch-case.
> Implement the ability for the user (the device developer) to add a separate callback for each state.

## How each point is done

| Point | In the code (`src/`) |
|---|---|
| Rewrite the state machine as a switch-case | `button_fsm.c`: the four flags of the lecture code (`pending_single`, `second_press`, `long_emitted`, `stuck_emitted`) are replaced with one field `state` of type `button_state_t` and a `switch (s->state)` with a `case` per state. The debouncer (`raw_prev`, `raw_change_us`, `stable_pressed`) stays as it was and only hands the switch two stable edges, `pressed_edge` and `released_edge` |
| A separate callback for each state | read as each **event** the device developer sees: SHORT, DOUBLE, LONG, STUCK (the state machine's own states are internal). `button_fsm_on()` stores one callback and its `ctx` per event inside the state machine (`handlers[]`, `handler_ctx[]` in `button_fsm_t`); an event without a callback is ignored; `button_fsm_poll()` no longer takes a callback. `main.c` registers four small handlers |

The states, each with what it waits for:

| State | Meaning | Leaves on |
|---|---|---|
| `IDLE` | released, nothing pending | stable press → `HELD` |
| `HELD` | first press | stable release → `WAIT_SECOND`; 2 s held → LONG, `LONG_HELD` |
| `WAIT_SECOND` | released after a short press, the 350 ms double-click window is open | stable press → `SECOND_HELD`; 350 ms passed → SHORT, `IDLE` |
| `SECOND_HELD` | the second press of a double | stable release → DOUBLE, `IDLE`; 2 s held → LONG, `LONG_HELD` |
| `LONG_HELD` | LONG reported, still held | stable release → `IDLE`; 7 s held → STUCK, `STUCK_HELD` |
| `STUCK_HELD` | STUCK reported, still held | stable release → `IDLE` |

The device developer's side, in `main.c`:

```c
button_fsm_init(&button, esp_timer_get_time());
button_fsm_on(&button, BUTTON_SHORT,  on_short,  NULL);
button_fsm_on(&button, BUTTON_DOUBLE, on_double, NULL);
button_fsm_on(&button, BUTTON_LONG,   on_long,   NULL);
button_fsm_on(&button, BUTTON_STUCK,  on_stuck,  NULL);
...
button_fsm_poll(&button, raw, esp_timer_get_time());   // every 10 ms
```

## Wiring

As in step 00:

| Part | GPIO |
|---|---|
| Button (other leg → 3.3 V, internal pull-down) | 4 |
| LED (330 Ω, cathode → GND) | 5 |

## Checked

- Every commit builds with 0 warnings (`-Wall -Wextra`, espressif32 7.1.3).
- A missing `case` is caught: with `case STUCK_HELD` removed (on a copy), the compiler reports
  `enumeration value 'STUCK_HELD' not handled in switch [-Wswitch]`. That is why the switch has no `default`.
- Not run on the board. The behaviour was compared with the lecture code by reading both.

**Notes**
- Same gestures and timings as the lecture code, with one edge: if the second press becomes stable in the same 10 ms
  poll in which the 350 ms window ends, the lecture code reports SHORT and starts a new press, while this version
  reports DOUBLE. The window is in effect up to 360 ms.
- As in the lecture code, a second press held past 2 s gives LONG only: the first short press is dropped.
- Register the callbacks **after** `button_fsm_init()`: init clears the whole struct, the callbacks included, and a
  button registered before it would silently do nothing.
- `button_fsm_on()` checks the event index: a C array neither grows nor throws, so a write to `handlers[7]` would
  silently overwrite the fields after the array.
- The state names (`IDLE`, `HELD`, …) have no prefix and live in a header other files include; there is no clash in
  ESP-IDF's headers today, but a prefix (`BUTTON_STATE_IDLE`) would be safer.
- The STM32 snippet of step 00 uses the lecture API (the callback passed to `button_fsm_poll()`); with this version it
  would register its `report()` through `button_fsm_on()` instead.
