# 00 — Homework: an exhaust fan on a hardware timer (ESP32-S3)

The lesson 2.5 homework on the ESP32-S3, in C on ESP-IDF (the assignment allows the Arduino framework or ESP-IDF).
A relay stands in for the fan. A general-purpose hardware timer (GPTimer) switches it from its interrupt callback;
the main loop only logs the state and feeds the task watchdog. One commit per change, so the **commits are the steps**.

## Assignment

The teacher's text in the LMS, translated in full.

**Homework: Timers**

The code and the result of running it on GitHub; the link to the repository in the LMS. Use the Arduino Framework or
ESP-IDF.

**Task**

Implement control of an exhaust fan that turns on every hour for 15 minutes and then turns off automatically.

- Fan = a motor or a relay
- The timers must work independently of the code in the loop
- Do not use `delay()` to implement the task
- The cycle repeats without the participation of `loop()` / a task
- The run time and the period are set by variables or macros

For debugging you may:

- replace 1 hour with a few seconds
- replace 15 min with a proportionally shorter interval

**Optional**

- Watchdog
- State logging
- Protection against a repeated start
- LED indicators that duplicate the current state

## How each point is done

| Point | Commit | In `src/main.c` |
|---|---|---|
| Fan = a relay | ba62dec | GPIO 7 drives the relay stage (see Wiring); 1 = fan on |
| Timers independent of the loop | ba62dec | a GPTimer at 1 MHz (1 tick = 1 µs) with an alarm; the alarm callback `on_alarm()` runs in the timer's interrupt and switches GPIO 7. In this commit `app_main()` returns after `gptimer_start()` and the relay keeps switching |
| No `delay()` | all | the timing comes from the timer's counter only. The one `vTaskDelay()` (from fb32552 on) paces the logging loop, not the fan |
| The cycle repeats without `loop()` / a task | ba62dec, c8c828f | `flags.auto_reload_on_alarm` restarts the counter from 0 in hardware at each alarm, and `on_alarm()` re-arms the alarm with `gptimer_set_alarm_action()` for the new state's length: `FAN_ON_S` after switching on, `FAN_PERIOD_S - FAN_ON_S` after switching off. No task takes part |
| Run time and period in macros | c8c828f | `FAN_PERIOD_S` and `FAN_ON_S`, in seconds, converted to ticks with `* 1000000ULL` |
| Optional: state logging | fb32552 | the callback only flips `volatile bool is_relay_on`; a loop in `app_main()` reads it once per pass and prints `fan ON` / `fan OFF` when it changes |
| Optional: watchdog | d686e3b, 51fb993 | d686e3b: `esp_task_wdt_add(NULL)` puts the logging loop under the task watchdog, `esp_task_wdt_reset()` feeds it on every pass. 51fb993: `esp_task_wdt_reconfigure()` with `trigger_panic = true` makes a timeout reboot the board (5 s, both idle tasks still watched), and `app_main()` prints `started after a task watchdog reset` when `esp_reset_reason()` says so |
| Optional: protection against a repeated start | — | not done |
| Optional: LED indicators | — | not done as separate LEDs; the relay module's own green LED lights while its input is driven |

**The times in the code are the debugging values the assignment allows:** `FAN_PERIOD_S 8` and `FAN_ON_S 2` (8 s to
2 s, the same ratio as 60 min to 15 min). For the real cycle set `FAN_PERIOD_S 3600` and `FAN_ON_S 900`; nothing else
changes (the tick count is 64-bit, see Notes). The real values were not run on the board.

## Wiring

The relay stage of lesson 2.2 (`../../2.2-passive-active-loads/00-homework-relay-delay-esp32/`):

| Part | Connection |
|---|---|
| GPIO 7 | 10 kΩ → BC547 base |
| BC547 emitter | GND |
| BC547 collector | relay module `IN`, plus a 10 kΩ pull-up to +5 V |
| Relay module `VCC` / `GND` | +5 V / GND from the breadboard supply; GND shared with the ESP32 |

GPIO 7 high → the transistor conducts → `IN` goes low → the relay turns on (this module switches on a low `IN`; a
3.3 V GPIO straight on `IN` can't turn it off, see 2.2).

## Checked on the board

Flashed over the UART port, the log read at 115200 baud; the times are from the board's reset, by the computer's clock:

```
 0.28 s  I (277) main_task: Calling app_main()
 6.29 s  fan ON
 8.29 s  fan OFF
14.29 s  fan ON
16.29 s  fan OFF
```

- With `FAN_PERIOD_S 8` and `FAN_ON_S 2` the relay clicks on after 6 s off, stays on 2 s, and the cycle repeats.
- ba62dec: the log shows `Returned from app_main()` and the relay kept switching every 2 s: the timer works with no task.
- With the task watchdog on (d686e3b, 51fb993): a 12 s run printed no `task_wdt` line and no reboot.
- **Not tried on the board:** the reboot itself on a watchdog timeout and the `started after a task watchdog reset`
  line. They follow from the code and the configuration only.

Build: every commit builds with 0 warnings (`-Wall -Wextra`; espressif32 7.1.3, ESP-IDF 6.1.0, GCC 15.2). The final
code uses 15488 bytes of RAM and 184085 bytes of flash.

## Notes

- **No `printf` in the callback.** `on_alarm()` runs inside the timer interrupt; printing there is slow (about 87 µs
  per character at 115200 baud) and takes locks an interrupt must not take. The callback only changes a variable, and
  the loop prints.
- **`1000000ULL`, not `1000000`.** One hour is 3600 × 1 000 000 = 3.6 × 10⁹ ticks; in a 32-bit `int` that
  multiplication overflows (the limit is about 2.1 × 10⁹). `ULL` makes it 64-bit, and the alarm count is a `uint64_t`.
- **No drift from the interrupt's delay.** The counter goes back to 0 in hardware at the alarm
  (`auto_reload_on_alarm`), so the time the CPU needs to enter the callback doesn't add up from cycle to cycle.
- **Order of the GPTimer calls:** `gptimer_new_timer` → `gptimer_register_event_callbacks` (before
  `gptimer_enable`, as the driver requires) → `gptimer_set_alarm_action` → `gptimer_enable` → `gptimer_start`. The
  driver allows `gptimer_set_alarm_action` inside the callback.
- **The task watchdog only logs by default:** this project's `sdkconfig` has `# CONFIG_ESP_TASK_WDT_PANIC is not set`.
  `esp_task_wdt_reconfigure()` turns the panic on from the code. `idle_core_mask = (1u << 0) | (1u << 1)` keeps both
  idle tasks watched, as before.
- **The interrupt watchdog is on by default** (`CONFIG_ESP_INT_WDT`, 300 ms): a callback that hangs reboots the chip
  without any code of ours.
- **No warnings for the callback's unused parameters** (`edata`, `user_ctx`): ESP-IDF adds `-Wno-unused-parameter`
  after `-Wextra`.
- **ESP-IDF 6.1 has only the GPTimer driver** (`driver/gptimer.h`); examples with the old `driver/timer.h` don't build.

## Working with the step

- **Build:** `pio run -d module-2/2.5-timers/00-homework-exhaust-fan-esp32` (or open it in the workspace and use the
  PlatformIO buttons).
- **Flash and log:** connect the UART port (the CH343P one), then Upload and Monitor at 115200 baud. Close the
  monitor before uploading: it holds the port, and the upload fails.
- **Walk the steps:** `git log -- module-2/2.5-timers/00-homework-exhaust-fan-esp32`, then check out a commit to build
  that version.
