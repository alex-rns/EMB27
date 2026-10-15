# Module 1 — technical notes (measurements, circuits, basic electronics)

> Claude Code loads this file when working inside `module-1/`. Project rules live in `../CLAUDE.md`.
> Update after every lesson of this module.

## Lessons → folders → status

| Lesson | Topic | Folder | Status |
|---|---|---|---|
| 1.1 | Measuring current and voltage | — | lecture only |
| 1.2 | Can you burn an LED? Resistance and voltage | — | lecture only |
| 1.3 | ESP32: first program, Blink | `1.3-esp32-blink` | ✅ finished (base + 4 extras) |
| 1.4 | GPIO and pinout, reading the datasheet | `1.4-gpio-pinout` | ✅ finished: `00`, `01` (lecture), `03`–`05` (homework), `06` (extra: interrupts); `02` unused |
| 1.5 | Signals and a button (oscilloscope) | `1.5-signals-button` | ✅ finished: `00` (lecture: debounce), `01` (homework: interrupt counter vs analyzer) |
| 1.6 | Intro to the ADC: reading light | — | — |
| 1.7 | Mini project: light sensor controls an LED | — | — |
| 1.8 | Debrief: typical mistakes (LED, polarity, wiring) | — | — |
| 1.9 | Basic soldering | — | — |

Module 1 has no flexible slot (those are in modules 2–6).

## Hardware in this module

- ESP32-S3 DevKitC-1 clone, N16R8 (see the root CLAUDE.md). Which GPIOs drive the onboard LEDs (47, 48) is still
  to verify: 1.3 uses both for external LEDs.
- LEDs (red, blue, white, green), 220 Ω and 10 kΩ resistors, tactile button, breadboard. Coming:
  light sensor (1.6–1.7), multimeter (1.1), soldering iron (1.9).
- Logic analyzer (1.5): 8-channel 24 MHz Saleae Logic clone, digital only, fixed input threshold. The case labels
  the channels 1–8, Logic 2 and PulseView call them D0–D7. Its GND goes to the board's GND.
- Pins used so far: 1.3: 47, 48, 21. 1.4: 45 (lecture, strapping pin on purpose), 15, 16, 21, 0 (homework).
  1.5: 4, 5, 6, 7 (lecture), 16 (homework).
- GPIO 0 (BOOT) has an external pull-up on this board; Wokwi's DevKitC-1 model doesn't, so a Wokwi circuit
  that reads BOOT with `INPUT` needs a 10 kΩ from GPIO 0 to 3V3 in `diagram.json`.
- Avoid: 19/20 (native USB — the board drops off the port), 0/3/45/46 (strapping),
  26–37 (flash/PSRAM), 43/44 (UART0).

## What to watch for (common mistakes, including in AI-generated code)

- **GPIO:** input vs output, pull-up/pull-down, push-pull vs open-drain.
  Watch for a pin mode that was never set, a forgotten pull-up.
- **Button (1.5):** a button read as plain `INPUT` with no pull resistor gives a floating pin,
  which reads random values. Contacts bounce on releases too, so a `FALLING` counter counts releases;
  a debounce must filter both directions.
- **ADC (1.6–1.7):** ESP32 `analogRead()` is 12-bit (0–4095), not 10-bit (0–1023) like an Arduino Uno;
  generated code often assumes 1023. The ESP32 ADC is non-linear near the rails, and attenuation sets the
  input range. Check which ADC unit a pin belongs to: prefer ADC1, because ADC2 is shared with Wi-Fi.
- **LEDs:** polarity, a resistor from Ohm's law, max current per pin (datasheet).
- **Datasheet:** pin functions, strapping pins, electrical characteristics.

## Lesson notes

**1.3 — ESP32 Blink, homework "police light".**
The preprocessor cuts code under `#if`, so all flags at `0u` give an empty `loop()`; the cut code never
reaches flash (a runtime `if` keeps both branches).
`pinMode` after `ledcAttachPin` detaches PWM from the pin.
220 Ω at 3.3 V: red ≈ 5.9 mA, blue ≈ 1.4 mA. Blue and white are dimmer because they get less current
(Vf ≈ 3 V leaves ~0.3 V across the resistor), not because they need more. With so little headroom the
Vf spread between parts (≈ 2.8–3.2 V) changes the current a lot. Break-before-make.
Variable speed: `#define` → variable, global because `loop()` runs again and again;
`int`, not `uint32_t`, because an underflow would turn into a 49-day `delay`.
Two LEDs on one pin: 3V3→LED→pin (active-low) + pin→LED→GND — source/sink, push-pull.
`INPUT` is high-Z: the pin neither sources nor sinks current, it only senses the voltage. Step 05 then
goes dark because the only path left (3V3 → blue → red → GND) needs ~5 V.
Current flows only where there is a voltage difference; through a diode only from + to −.
Two push-pull outputs driving opposite levels short each other (hence open-drain in I2C).
Steps: `1.3-esp32-blink/00…05` — lecture demo, base, 4 extras; each an ordinary PlatformIO project.
Steps 01–03 were reconstructed afterwards (they were only intermediate edits of one `main.cpp`).

**1.4 — GPIO and pinout, homework "two LEDs, a button and BOOT".**
Pull-down: resistor to GND, button to 3V3, pressed = 1. Pull-up: resistor to 3V3, button to GND, pressed = 0.
The resistor sets the idle level, the button pulls the pin to the other rail; a button to the same rail
changes nothing. `INPUT_PULLUP`/`INPUT_PULLDOWN` switch on the chip's ~45 kΩ resistor; plain `INPUT` switches
it off (Arduino 2.0.17 `pinMode`), so the pin floats unless something on the board holds it.
GPIO 0 (BOOT) has an external pull-up on this board, so `INPUT` works; Wokwi's DevKitC-1 model lacks it and
reads GPIO 0 as LOW. Wokwi also never shows a floating pin's noise: it reads 0.
A Serial Monitor open on the UART port (CH343P) can hold GPIO 0 LOW through the DTR/RTS auto-download
transistors; a program reading BOOT then acts as if it were held.
Strapping pins: GPIO 0 picks the boot mode, GPIO 45 the flash voltage. Holding them at the wrong level
during a reset can keep the board from booting; an external pull-up on 45 does that at every reset.
Breadboard: a tactile button across the gap joins its rows only when pressed; legs in one row are always
connected. A pin in the 3V3 row reads 1 all the time.
`Serial` is UART0 (the UART USB-C port); `monitor_speed` must match `Serial.begin()` or the monitor shows garbage.
`delay()` blocks: buttons read once per cycle miss short presses (up to 0.4 s / 2 s). Interrupts on `FALLING`
react to the edge, not the level: the press is latched in hardware even during `delay()`. Handlers are
short (no `delay`, no `Serial`) and share data through `volatile` variables. `IRAM_ATTR` is optional in core
2.0.17: `attachInterrupt` doesn't register IRAM-safe interrupts, so they wait while the flash is busy (1.5).
Bounce calls a handler several times; harmless while it sets a value, not when it toggles.
Steps: `1.4-gpio-pinout/00, 01` lecture (pull-down, pull-up), `03–05` homework in stages, `06` extra (interrupts).

**1.5 — Signals and a button, homework "button bounce on a logic analyzer".**
The analyzer records only HIGH/LOW, at 24 MS/s (41.7 ns per sample); no analog shape, no rise time.
Logic 2 and PulseView each load their own firmware into it: close one and replug before using the other.
PulseView starts at 20 kHz after a restart, too coarse for bounce: check the rate before every run.
Bounce of the kit's tactile button, measured: on releases only, 0.07–0.4 ms in the homework, up to ~1.2 ms
seen earlier, pulses down to 0.2 µs. Not the 5–20 ms the lesson text gives.
`FALLING` counter vs analyzer over 10 presses: 16 falling edges, 36 counts. The counter over-counts on releases:
+1 even on a release the analyzer saw as clean, +4…+6 on a bouncy one. Cause not verified: the chip and the
analyzer have different input thresholds, and the analyzer can't see a pulse shorter than one sample.
The firmware also misses pulses: under ~1 µs they vanish, from ~10 µs on it sees them (lecture capture).
The interrupt runs microseconds after the edge and reads the pin; two edges before it runs leave one pending flag.
Lecture debounce: the `CHANGE` ISR only records (level, time, count), `loop()` decides. The quiet window
restarts on every edge in both directions; a level is accepted after 30 ms of silence (29–30 ms measured,
`millis()` counts whole ms). A window also swallows real events shorter than it (a 24.2 ms gap merged).
Shared ISR data: `volatile` + a `portMUX` spinlock, one snapshot under the lock, `now` read last under it;
`now - last >= interval` survives the 49.7-day `millis()` wrap, `now >= last + interval` doesn't.
`attachInterrupt` goes last in `setup()`: the unlocked initial writes must not race the ISR.
`Serial` from an ISR: its UART lock is a mutex with an infinite wait, so an ISR that interrupts a print
deadlocks. `ARDUINO_ISR_ATTR` is empty in core 2.0.17. `Serial` has no software TX buffer here: a call
returns once the text is in the 128-byte UART FIFO.
`monitor_filters = time` stamps every monitor line, which maps Serial output to edges in a capture
(lines arrive 29–39 ms after the edge).
Steps: `1.5-signals-button/00` lecture (debounce on four channels), `01` homework (counter vs analyzer).

## Known issues / open

1. 1.3: add "▶ Simulate in Wokwi" links to each step README and the lesson's steps table once
   the steps are published on wokwi.com.
2. 1.3 step 04: the README says the flash length moves "between 100 ms and 15 ms", but the code
   reaches 10 ms: the bound is checked after `flashMs += stepMs`, and 100 − 15 is not a multiple of 10
   (100, 90, …, 20, 10 → flip). Open: fix the code or the README.
3. 1.4: check on the board which GPIOs drive the onboard LEDs (course: 48 = blue SYS-LED, 47 = WS2812).
   1.3 drives external LEDs on both pins, so the onboard LEDs may blink along.
