# Module 1 — technical notes (measurements, circuits, basic electronics)

> Claude Code loads this file when working inside `module-1/`. Project rules live in `../CLAUDE.md`.
> Update after every lesson of this module.

## Lessons → folders → status

| Lesson | Topic | Folder | Status |
|---|---|---|---|
| 1.1 | Measuring current and voltage | — | lecture only |
| 1.2 | Can you burn an LED? Resistance and voltage | — | lecture only |
| 1.3 | ESP32: first program, Blink | `1.3-esp32-blink` | ✅ base + 4 extras saved |
| 1.4 | GPIO and pinout, reading the datasheet | `1.4-gpio-pinout` | ⏳ experiment project (wizard template) |
| 1.5 | Signals and a button (oscilloscope) | — | — |
| 1.6 | Intro to the ADC: reading light | — | — |
| 1.7 | Mini project: light sensor controls an LED | — | — |
| 1.8 | Debrief: typical mistakes (LED, polarity, wiring) | — | — |
| 1.9 | Basic soldering | — | — |

Module 1 has no flexible slot (those are in modules 2–6).

## Hardware in this module

- ESP32-S3 DevKitC-1 clone, N16R8 (see the root CLAUDE.md). Which GPIOs drive the onboard LEDs (47, 48) is still
  to verify: 1.3 uses both for external LEDs.
- LEDs (red, blue, white), 220 Ω resistors, breadboard. Coming: button (1.5),
  light sensor (1.6–1.7), multimeter (1.1), soldering iron (1.9).
- Pins used so far: 47, 48, 21.
- Avoid: 19/20 (native USB — the board drops off the port), 0/3/45/46 (strapping),
  26–37 (flash/PSRAM), 43/44 (UART0).

## What to watch for (common mistakes, including in AI-generated code)

- **GPIO:** input vs output, pull-up/pull-down, push-pull vs open-drain.
  Watch for a pin mode that was never set, a forgotten pull-up.
- **Button (1.5):** a button read as plain `INPUT` with no pull resistor gives a floating pin,
  which reads random values. Contacts bounce (debounce comes in module 2).
- **ADC (1.6–1.7):** ESP32 `analogRead()` is 12-bit (0–4095), not 10-bit (0–1023) like an Arduino Uno;
  generated code often assumes 1023. The ESP32 ADC is non-linear near the rails, and attenuation sets the
  input range. Check which ADC unit a pin belongs to: prefer ADC1, because ADC2 is shared with Wi-Fi.
- **LEDs:** polarity, a resistor from Ohm's law, max current per pin (datasheet).
- **Datasheet:** pin functions, strapping pins, electrical characteristics.

## Lesson notes

**1.3 — ESP32 Blink, homework "police light".**
The preprocessor cuts code under `#if`, so all flags at `0u` give an empty `loop()`.
`pinMode` after `ledcAttachPin` detaches PWM from the pin.
220 Ω resistor: blue and white are dimmer (Vf ≈ 3 V at 3.3 V). Break-before-make.
Variable speed: `#define` → variable, global because `loop()` runs again and again;
`int`, not `uint32_t`, because an underflow would turn into a 49-day `delay`.
Two LEDs on one pin: 3V3→LED→pin (active-low) + pin→LED→GND — source/sink, push-pull.
Current flows only where there is a voltage difference; through a diode only from + to −.
Two push-pull outputs driving opposite levels short each other (hence open-drain in I2C).
Steps: `1.3-esp32-blink/00…05` — lecture demo, base, 4 extras; each an ordinary PlatformIO project.
Steps 01–03 were reconstructed afterwards (they were only intermediate edits of one `main.cpp`).

## Known issues / open

1. 1.3: add "▶ Simulate in Wokwi" links to each step README and the lesson's steps table once
   the steps are published on wokwi.com.
2. 1.3 step 04: the README says the flash length moves "between 100 ms and 15 ms", but the code
   reaches 10 ms: the bound is checked after `flashMs += stepMs`, and 100 − 15 is not a multiple of 10
   (100, 90, …, 20, 10 → flip). Open: fix the code or the README.
3. 1.4: the PlatformIO template still has `int result = myFunction(2, 3);` (`-Wunused-variable`).
4. 1.4: check on the board which GPIOs drive the onboard LEDs (course: 48 = blue SYS-LED, 47 = WS2812).
   1.3 drives external LEDs on both pins, so the onboard LEDs may blink along.
