# Lesson 1.3 — ESP32: first program (Blink). Homework: police light

[← Module 1](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi

**Assignment:** drive a red and a blue LED from two GPIO pins through resistors
and make them blink alternately in an endless loop, like a police light.
Optional extras: a more complex pattern, a third LED, variable speed,
and fewer pins — both LEDs from a single GPIO.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-blink](00-lecture-blink/) | lecture demo | onboard RGB LED, `#if` feature flags, LEDC PWM |
| 01 | [base-alternate](01-base-alternate/) | base | two GPIO outputs blinking alternately |
| 02 | [extra-double-flash](02-extra-double-flash/) | extra | police-style double flash, timings as named constants |
| 03 | [extra-third-led](03-extra-third-led/) | extra | third LED, two LEDs lit together |
| 04 | [extra-variable-speed](04-extra-variable-speed/) | extra | speed ramps up and down; state kept across `loop()` calls |
| 05 | [extra-one-gpio](05-extra-one-gpio/) | extra | two LEDs on one pin: current source vs sink |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp`, and a Wokwi circuit.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 05-extra-one-gpio`.
- **Simulate:** build the step first, then open its `diagram.json` and press ▶.
- **Wiring:** every LED goes through its own 220 Ω resistor.
