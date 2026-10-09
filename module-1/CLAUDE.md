# Module 1 context — measurements, circuits, basic electronics

> Claude Code loads this file when working inside `module-1/`.
> Shared rules (who I am, how to work with me, conventions) live in `../CLAUDE.md`.
> Update after every lesson of this module.

## Lessons → folders → status

| Lesson | Topic | Folder | Status |
|---|---|---|---|
| 1.1 | Measuring current and voltage | — | lecture only |
| 1.2 | Can you burn an LED? Resistance and voltage | — | lecture only |
| 1.3 | ESP32: first program, Blink | `1.3-esp32-blink` | ✅ base + 4 extras |
| 1.4 | GPIO and pinout, reading the datasheet | `1.4-gpio-pinout` | ⏳ template only, homework not given yet |
| 1.5 | Signals and a button (oscilloscope) | — | — |
| 1.6 | Intro to the ADC: reading light | — | — |
| 1.7 | Mini project: light sensor controls an LED | — | — |
| 1.8 | Debrief: typical mistakes (LED, polarity, wiring) | — | — |
| 1.9 | Basic soldering | — | — |

Module 1 has no flexible slot (those are in modules 2–6).

## Hardware in this module

- ESP32-S3-DevKitC-1, looks like rev v1.1: onboard RGB on GPIO 38, GPIO 48 is free.
- LEDs (red, blue, white), 220 Ω resistors, breadboard. Coming: button (1.5),
  light sensor (1.6–1.7), multimeter (1.1), soldering iron (1.9).
- Pins used so far: 47, 48, 21.
- Avoid: 19/20 (native USB — the board drops off the port), 0/3/45/46 (strapping),
  26–37 (flash/PSRAM), 43/44 (UART0).

## Interview questions / what to catch in AI code for these topics

- **GPIO:** input vs output, pull-up/pull-down, push-pull vs open-drain.
  Catch: a pin mode that was never set, a forgotten pull-up.
- **Button (1.5):** a button read as plain `INPUT` with no pull resistor gives a floating pin,
  which reads random values. Contacts bounce (debounce comes in module 2).
- **ADC (1.6–1.7):** ESP32 `analogRead()` is 12-bit (0–4095), not 10-bit (0–1023) like an Arduino Uno.
  AI code often assumes 1023. The ESP32 ADC is non-linear near the rails, and attenuation sets the
  input range. Check which ADC unit a pin belongs to: prefer ADC1, because ADC2 is shared with Wi-Fi.
- **LEDs:** polarity, a resistor from Ohm's law, max current per pin (datasheet).
- **Datasheet:** pin functions, strapping pins, electrical characteristics.

## Lesson summaries

**1.3 — ESP32 Blink, homework "police light" (2026-10-09/10).**
Board connected, port and VID:PID explained, serial monitor (ESP-IDF factory demo).
The preprocessor cuts code under `#if`, so all flags at `0u` give an empty `loop()`.
`pinMode` after `ledcAttachPin` detaches PWM from the pin.
220 Ω resistor: blue and white are dimmer (Vf ≈ 3 V at 3.3 V). Break-before-make.
Police light: 3 LEDs, double flashes, variable speed (`#define` → variable, global
because `loop()` runs again and again; `int`, not `uint32_t`, because of underflow → a 49-day `delay`).
**Two LEDs on one pin:** 3V3→LED→pin (active-low) + pin→LED→GND. Source/sink, push-pull.
Key idea I **formulated myself**: current flows only where there is a voltage *difference*;
the pin doesn't "decide" anything, it just becomes either plus or minus. Current flows through a diode only from + to −.
I know that two outputs, one HIGH and one LOW, wired together make a short circuit (hence open-drain in I2C).
Tasks: `1.3-esp32-blink/tasks/00…05` — lecture demo, base, 4 extras; each with code, circuit and README.
Tasks 01–03 were reconstructed from the session (they were only intermediate edits of main.cpp).

## Open threads

1. 1.3: refactor the police light into a bit-pattern table (e.g. `0b011`) — a bridge to the
   bits lesson and to `uint8_t`. Proposed, not done.
2. 1.3: I'm publishing each task to my wokwi.com account. When I send the URLs, add a
   "▶ Simulate in Wokwi" link to each task README and to the lesson's task table.
   Decided 2026-10-10: links only, no static images.
3. 1.4: the PlatformIO template still has `int result = myFunction(2, 3);`
   (`-Wunused-variable`). Replace it when the homework starts.
