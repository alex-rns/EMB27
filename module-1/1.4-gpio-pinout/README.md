# Lesson 1.4 — GPIO and pinout, reading the datasheet. Homework: two LEDs, a button and BOOT

[← Module 1](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi

**Assignment:** two LEDs on free GPIOs (for example 15 and 16), each through a 220 Ω resistor; an external
tactile button on a free GPIO (for example 21); the onboard BOOT button (GPIO 0) as the second button.
The program runs one of two modes; the choice is latched until the other button is pressed, and mode 1 is
active at start:
- mode 1, selected by the external button: both LEDs turn on and off together, `delay(200)`;
- mode 2, selected by BOOT: the LEDs alternate like a railway crossing, `delay(1000)`.

LED pins are `OUTPUT`, the external button is `INPUT_PULLUP`, BOOT is `INPUT` (the board has an external
pull-up on GPIO 0).

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-pulldown](00-lecture-pulldown/) | lecture demo | input with a pull-down, a button to 3V3, values in Serial |
| 01 | [lecture-pullup](01-lecture-pullup/) | lecture demo | the same turned around: a pull-up, a button to GND, pressed = LOW |
| 03 | [base-mode1-sync](03-base-mode1-sync/) | base | mode 1 while the button is held: both LEDs together, 200 ms |
| 04 | [base-mode2-alternate](04-base-mode2-alternate/) | base | mode 2 while BOOT is held: the LEDs in turn, 1000 ms; BOOT as `INPUT` |
| 05 | [base-combined](05-base-combined/) | base | the full homework: both modes, latched in a global `mode`, mode 1 at start |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp`, and a Wokwi circuit.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 00-lecture-pulldown`.
- **Simulate:** build the step first, then open its `diagram.json` and press ▶.
- **Serial:** steps that print use 115200 baud; read them on the board's UART USB-C port.
