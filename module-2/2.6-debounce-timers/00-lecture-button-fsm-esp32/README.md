# 00 — Lecture: a button state machine (the teacher's demo)

The lecture demo of lesson 2.6, as the teacher shared it after class (a project with `button_fsm/`, `esp-idf/` and
`stm32_hal/`), unchanged. ESP-IDF in C.

- **Polling task:** `app_main()` reads GPIO 4 every 10 ms (`xTaskDelayUntil`, so the period does not drift by the
  length of the work) and gives `button_fsm_poll()` the raw level and `esp_timer_get_time()`.
- **`button_fsm.c`:** debounces the level (a level held for 30 ms becomes the stable one) and recognises four
  gestures: **SHORT** (reported 350 ms after the release, once no second press came), **DOUBLE** (two short presses,
  the second within 350 ms), **LONG** (held 2 s, reported once while still held), **STUCK** (held 7 s). It knows no
  pins and no clock: the caller gives it the level and the time, and it reports through one callback.
- **`main.c`:** `on_button_event()` toggles the LED on GPIO 5 on SHORT, turns it on on LONG, off on DOUBLE and STUCK,
  and logs each event.
- **`stm32_hal/main.c`:** the teacher's integration snippet for the Black Pill (PB5 button to 3V3 with the pull-down,
  PB6 LED), reusing the same `button_fsm.c/.h`. Not a project and not built.

| Part | GPIO |
|---|---|
| Button (other leg → 3.3 V, internal pull-down) | 4 |
| LED (330 Ω, cathode → GND) | 5 |

## Checked

Builds with 0 warnings (`-Wall -Wextra`, espressif32 7.1.3). Not run on the board.

**Notes**
- The button is active high: released reads 0 through the internal pull-down, pressed reads 1. The other circuits of
  the course so far are active low (pull-up, button to GND).
- The state machine keeps its state in six `bool` flags and has no state variable; the extra homework (step 01)
  rewrites it as a `switch-case` over explicit states.
- A second press held past 2 s gives only LONG: LONG drops the pending short press, and the release after a LONG
  reports nothing.
- STM32 snippet: `HAL_GetTick()` is a 32-bit millisecond counter that wraps after about 49.7 days, while
  `button_fsm.h` expects a monotonic time. `(int64_t)HAL_GetTick()*1000` casts before multiplying, so the product
  itself does not overflow. The snippet passes `&counters` as `ctx`, which `report()` does not use yet, and calls
  `HAL_Delay(10)` in `app_button_update()`.
