# Module 1.3 — GPIO: police light

**Board:** ESP32-S3-DevKitC-1 · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi

**Assignment:** drive a red and a blue LED from two GPIO pins through resistors
and make them blink alternately in an endless loop, like a police light.
Optional extras: a more complex pattern, a third LED, variable speed,
and fewer pins — both LEDs from a single GPIO.

## Tasks

| # | Task | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-blink](tasks/00-lecture-blink/) | lecture demo | onboard RGB LED, `#if` feature flags, LEDC PWM |
| 01 | [base-alternate](tasks/01-base-alternate/) | base | two GPIO outputs blinking alternately |
| 02 | [extra-double-flash](tasks/02-extra-double-flash/) | extra | police-style double flash, timings as named constants |
| 03 | [extra-third-led](tasks/03-extra-third-led/) | extra | third LED, two LEDs lit together |
| 04 | [extra-variable-speed](tasks/04-extra-variable-speed/) | extra | speed ramps up and down; state kept across `loop()` calls |
| 05 | [extra-one-gpio](tasks/05-extra-one-gpio/) | extra | two LEDs on one pin: current source vs sink |

## Working with the tasks

Each task is its own PlatformIO environment built from `tasks/<task>/`.

- **Build / Upload:** pick the task's environment in the PlatformIO status bar,
  or set `default_envs` in `platformio.ini`.
- **Simulate:** build the task first, then open `tasks/<task>/diagram.json` and press ▶.
- **Wiring:** every LED goes through its own 220 Ω resistor.
