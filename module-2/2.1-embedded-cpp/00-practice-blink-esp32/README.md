# 00 — Practice: the homework tasks before the lecture

The four homework tasks done once before the lecture, as a self-check. The homework itself is a separate step.

- **Blink without `delay()`:** the LED on GPIO 7 toggles every 1000 ms, timed with `millis()` in `loop()`.
- **`enum class LedState { On, Off }`** and a `Led` class with `init()` and `set(LedState)`.
- **`struct Config`** with `static constexpr` members holds every number: pins, intervals, baud rate.
  `Config` lives in `include/config.h`, `LedState` and `Led` in `include/led.h`.
- **Loop timing:** `loop()` measures its own time with `micros()` and prints the average every 1000 iterations
  (`avg loop: 2.141 us`), Serial at 115200 baud.
- **Button modes:** a press on GPIO 9 sets a `volatile bool` flag in a minimal ISR; `loop()` clears it, ignores
  presses closer than 30 ms to the previous one, and switches the mode Blink → On → Off → Blink.

| Part | GPIO |
|---|---|
| LED (through a resistor, cathode → GND) | 7 |
| Button (other leg → GND, internal pull-up) | 9 |

## Checked on the board

- The LED blinks once a second; each press moves one mode on, Blink → On → Off → Blink.
- Average `loop()` time: 2.03–2.05 µs with the blink only, 2.12–2.16 µs with the button logic added.
- Build: RAM 19 496 bytes, Flash 278 725 bytes. `Serial` alone adds about 24 KB of flash and 360 bytes of RAM;
  the `enum class`, `constexpr` and `Led` abstractions add nothing.

**Notes**
- `Serial` is UART0: read it on the **UART** USB-C port (CH343P, vendor ID 1A86), with `monitor_speed = 115200`.
  At the monitor's default 9600 baud the lines come out as garbage.
- The timing stops before `Serial.print`, so the slow print stays out of the average.
- `micros()` counts whole microseconds, so one iteration reads as 2 or 3 µs; the average of 1000 gives the fraction.
