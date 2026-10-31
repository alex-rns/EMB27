# 00 — Homework: three LEDs in a non-blocking superloop (ESP32, ESP-IDF)

The lesson 2.3 homework on the ESP32-S3, in C on ESP-IDF: one `while (true)` loop in `app_main()` blinks three LEDs
at different rates from `esp_timer_get_time()`, without `vTaskDelay` or any waiting. One button pauses all blinking,
the other changes the interval of one LED. The git history of this folder shows the work one change per commit. The
same task on the Black Pill: [`00-homework-three-leds-stm32`](../00-homework-three-leds-stm32/).

## Assignment

The teacher's texts in the LMS, translated in full.

**Homework: the superloop and independent tasks**

Code and the result on GitHub; the link to the repository in the LMS.

Task: control several independent processes in one `loop()` without blocking. There are 3 LEDs:

- LED1 blinks every 200 ms
- LED2 blinks every 500 ms
- LED3 blinks every 1000 ms

Requirements:

- ❌ `delay()` is forbidden
- ❌ `while()` for waiting for time in `loop()` is forbidden
- ✅ `millis()` is allowed

**Additional homework**

- Make a non-blocking superloop where three LEDs blink at different frequencies.
- One button pauses all blinking.
- The second button changes the interval of one of them.
- Make the loop run in the minimal amount of time when none of the actions is needed.

## How each point is done

| Point | In the code (`src/main.c`) |
|---|---|
| Three LEDs at 200 / 500 / 1000 ms | `LEDS[3]`, an array of `LED_type_t` (pin, `half_period`, `last_change_timestamp`, `level`): GPIO 6 every 200 ms, GPIO 4 every 500 ms, GPIO 5 every 1000 ms. Each pass takes one `now_ms` and toggles an LED only when `now_ms - last_change_timestamp >= half_period` |
| No `delay()`, no `while()` waiting for time | no `vTaskDelay` (ESP-IDF's `delay()`); the only `while` is the superloop itself, and nothing inside it waits |
| `millis()` | ESP-IDF has no `millis()`: `esp_timer_get_time()` gives microseconds since boot as `int64_t`, `/ 1000` gives milliseconds |
| One button pauses all blinking | GPIO 7: each press flips `paused`, and while it is set the `for` over the LEDs runs no step, so every LED keeps the state it had |
| The second button changes one LED's interval | GPIO 15: each press moves `LEDS[2].half_period` (GPIO 5) to the next value, 1000 → 500 → 200 → 1000 ms |
| Minimal loop time when nothing is due | see [The idle pass](#the-idle-pass) |

Both buttons count a press on the change from released (1) to pressed (0), not while held, and each has its own
"previous level" variable, so a press on one never affects the other.

## The idle pass

When no button changed and no LED is due, one pass does only reads and comparisons: one `esp_timer_get_time()`, two
`gpio_get_level()`, two edge checks and three subtractions and comparisons, all false. No pin write, no print, no
waiting: `gpio_set_level()` and an interval change run only on the pass where they are due, and while paused the
`for` does no step. The pass time was not measured.

## Wiring

![Schematic](schematic.svg)

| Part | GPIO | Connection |
|---|---|---|
| LED1 (200 ms) | 6 | 220 Ω → LED anode (long leg); cathode → GND rail |
| LED2 (500 ms) | 4 | the same |
| LED3 (1000 / 500 / 200 ms) | 5 | the same |
| Pause button | 7 | button to the GND rail, on two diagonal legs; internal pull-up |
| Interval button | 15 | the same |
| GND | `G` | the breadboard's GND rail |

The board runs from USB. GPIO 4–7 and 15 are plain GPIOs (not strapping, USB, flash/PSRAM or UART pins).

## Notes

- **No toggle function.** ESP-IDF has only `gpio_set_level(pin, level)`, so every LED keeps its own `level` in the
  struct. `gpio_get_level()` can't replace it: on a pin set up as output only, its input is off and it reads 0.
- **Pin setup is ours.** `gpio_reset_pin()` (plain GPIO, input and output off), `gpio_set_direction()` and, for the
  buttons, `gpio_set_pull_mode(..., GPIO_PULLUP_ONLY)`: on the STM32 CubeMX generated this in `MX_GPIO_Init()`.
- **A pointer, not a copy.** `LED_type_t* led = &LEDS[led_idx];` points at the array element itself, so the updates
  through `led->` stay in the array. `LEDS[2].half_period` uses `.`: `LEDS[2]` is the element itself.
- **No onboard LED.** Toggling GPIO 48 lit nothing on this board: it has no plain LED there (likely a WS2812, which
  needs its own data protocol), so LED1 is on the breadboard too.
- **The task watchdog (known limit).** The superloop never yields, so FreeRTOS's idle task on CPU 0 never runs, and
  every 5 s the log prints `task_wdt: Task watchdog got triggered ... IDLE0`. It is a warning only: the board does
  not reset, and the LEDs keep their timing.
- **No debounce (known limit).** A press sometimes counts twice: the contacts bounce for a few milliseconds and the
  loop sees each bounce as a new press.
- **After a pause** the LEDs' timestamps are old, so every LED whose interval has passed toggles on the first pass
  after the pause ends; the rates are back to normal from the next toggle.

## Checked on the board

- The three LEDs blink at 200, 500 and 1000 ms.
- GPIO 7 freezes and resumes all blinking.
- GPIO 15 cycles GPIO 5's blinking 1000 → 500 → 200 → 1000 ms; the other two LEDs keep their rates.

Build: 0 warnings with `-Wall -Wextra`. RAM 15468 bytes (4.7 %), flash 174961 bytes (16.7 % of the 1 MB app
partition).

## Working with the step

An ordinary PlatformIO project on ESP-IDF, written in C: `platformio.ini`, `src/main.c`, the two `CMakeLists.txt`
the PlatformIO wizard makes for ESP-IDF, and `sdkconfig.defaults` (the full `sdkconfig.<env>` is generated at build
time and not tracked).

- **Build / Upload:** the PlatformIO buttons, or `pio run -d <this folder>` (`-t upload` to flash).
- **Serial Monitor:** 115200 baud (the watchdog warnings appear there).
