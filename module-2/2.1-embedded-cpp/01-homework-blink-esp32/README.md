# 01 — Homework: Embedded C++ blink on the ESP32

The homework of lesson 2.1: the classic Arduino Blink rewritten in Embedded C++, then the two optional tasks, a
superloop timer and a button on an interrupt. The git history of this folder shows the work one change per commit.

## Assignment

"Module 2.1. Homework: Embedded C++ on the ESP32" (translated in full).

**Format:** the code and the result on GitHub; the repository link in the LMS.

**Embedded C++ concepts:**
- `volatile`, `const` / `constexpr`, `static`;
- no dynamic memory and no heavy STL containers;
- the superloop architecture;
- safe work with peripherals (LED, button).

**1. Rewrite the "classic Arduino blink" in Embedded C++**
- Use `enum class` for the LED state (On / Off).
- Use `constexpr` for the pin number and the blink interval.
- Avoid global variables (except the minimum needed for state / the ISR).
- Don't use `delay()`; write non-blocking code in the superloop with `millis()`.
- Create a class `Led` with the methods `init()` and `set(LedState state)`.

**2. Use `static const` / `constexpr` for parameters**
- Create a config class or struct with `static const` / `constexpr` for further settings.
- Remove every "magic number" completely, moving it into constants.

**3. Measure the superloop's execution time (optional)**
- Measure the time of one `loop()` iteration without `delay()`.
- Print the average time or other figures to the Serial Monitor every 1000 iterations.

**4. Add a button handled through an interrupt (optional)**
- Connect a button to a GPIO and set up the interrupt (`attachInterrupt`).
- Use a `volatile bool` button flag to record the press event in the ISR.
- Change the LED mode on every press: blinking → always on → always off → blinking.
- Keep the ISR minimal (no `Serial.print` calls or delays).
- Handle all the mode switching and the debouncing in `loop()`.

## How each point is done

| Point | In the code |
|---|---|
| `enum class` for the LED state | `enum class LedState { On, Off }` in `include/led.h` |
| `constexpr` pin and interval | `Config::kLedPin = 7`, `Config::kBlinkIntervalMs = 1000` in `include/config.h` |
| Minimal globals | `Led led` (used by both `setup()` and `loop()`) and the ISR flag `btnPushed`; the loop's state lives in `static` locals inside `loop()` |
| No `delay()` | `loop()` toggles the LED when `nowMs - lastToggleMs >= Config::kBlinkIntervalMs`, and returns at once otherwise |
| `Led` class | `init()` sets the pin mode, `set(LedState)` writes `HIGH` or `LOW` |
| Config struct, no magic numbers | `struct Config` with `static constexpr` members: both pins, the blink interval, the baud rate, the report period, the debounce time |
| No dynamic memory, no STL | no `new`, `String` or containers in our code |
| Loop timing | `micros()` at the start and the end of the loop's work; the average is printed every `Config::kIterationsPerReport` (1000) iterations, after the measured part |
| Button on an interrupt | GPIO 9, `INPUT_PULLUP`, `attachInterrupt(..., FALLING)`; the ISR `onBtnPush()` only sets `btnPushed = true` |
| Modes and debouncing in `loop()` | `loop()` clears the flag, accepts the press only 30 ms (`Config::kDebounceMs`) or more after the previous one, and moves the mode on with `nextMode()`: Blink → On → Off → Blink |

## Wiring

| Part | GPIO |
|---|---|
| LED (through a resistor, cathode → GND) | 7 |
| Button (other leg → GND, internal pull-up) | 9 |

Serial: the **UART** USB-C port (CH343P, vendor ID 1A86), 115200 baud (`monitor_speed` in `platformio.ini`).

## Checked on the board

- The LED blinks once a second; each press moves one mode on, Blink → On → Off → Blink.
- Serial prints the average `loop()` time every 1000 iterations: `avg loop: 2.125` … `2.151 us`.
- Build: RAM 19 496 bytes, Flash 278 725 bytes, no warnings with `-Wall -Wextra`.

**Notes**
- **`IRAM_ATTR` on the ISR:** the ESP32 runs code from an external flash chip through a cache, and the cache is off
  while the flash is being written. An ISR left in flash could be called at that moment; in IRAM it is always there.
- **`volatile` on the flag:** without it the compiler may keep `btnPushed` in a register inside `loop()` and never
  see the ISR's write.
- **Pin setup in `init()`, not in a constructor:** the global `led` is constructed before `setup()` runs.
- **Time arithmetic:** `now - last >= interval` on `uint32_t` keeps working when `millis()` wraps after 49.7 days.
- **Timer resolution:** `micros()` counts whole microseconds, so one iteration reads as 2 or 3 µs; the average of
  1000 gives the fraction.
