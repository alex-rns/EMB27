# Lesson 1.6 — Intro to the ADC: reading light. Homework: an LDR voltage by formula vs calibration

[← Module 1](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi ·
**Logic analyzer:** 8-channel 24 MHz Saleae Logic clone (Logic 2)

**Assignment:** connect an LDR to the ESP32-S3 as a voltage divider. Every 100 ms read the raw ADC code with
`analogRead()`, compute the voltage as RAW / ADCmax × Uref with ADCmax = 4095 and Uref = 3.1 V, read the voltage
with `analogReadMilliVolts()`, and work out the relative error between the two in percent. Print the raw code,
both voltages and the error to the serial console as a table or structured text.

The circuit for both steps:

```
3V3 ── LDR ──┬── GPIO 4
             │
           10 kΩ
             │
            GND
```

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-smart-light](00-lecture-smart-light/) | lecture demo | a night light: moving average of 8 samples, an LED on GPIO 7 with hysteresis; recorded on the logic analyzer |
| 01 | [base-voltage-error](01-base-voltage-error/) | base | the homework: the straight-line formula against `analogReadMilliVolts()`, about −10 % on this board |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp` (step 00 also has `include/`),
and a Wokwi circuit.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 01-base-voltage-error`.
- **Simulate:** build the step first, then open its `diagram.json` and press ▶. Wokwi has only a photoresistor
  module, wired the other way round, and doesn't solve resistor dividers, so it runs the code but its numbers
  don't match the board.
- **Serial:** both steps print at 115200 baud; read them on the board's UART USB-C port.
- **Capture:** `00` has a Logic 2 capture (`capture.sal`) with the LDR node, the LED and the decoded UART.
