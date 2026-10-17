# Lesson 1.7 — Mini project: a sensor and an actuator. Homework: a twilight switch

[← Module 1](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi ·
**Logic analyzer:** 8-channel 24 MHz Saleae Logic clone (Logic 2)

**Assignment** ("Twilight switch on the ESP32-S3"): learn to read an analog signal from a voltage divider (ADC) and
to control an external load through a transistor switch (GPIO).

1. **Circuit**, built on the breadboard from the drawing. Input: the divider R1 (LDR) + R2 (10 kΩ) on the +3.3 V
   rail, its midpoint to "GPIO In" (ADC). Switch, matching 3.3 V to 5 V: "GPIO Out" through R3 (10 kΩ) to the base
   of a BC547B; the collector pulled up to +5 V through R4 (10 kΩ) and connected to the relay module's IN.
   Output: any safe low-voltage load (an LED, a 5 V / 12 V LED strip) on the relay's NO contact.
2. **Program**, three steps in the main loop: read the ADC from GPIO In (0…4095); compare it with two thresholds:
   below `THRESHOLD_DARK` (dark) → HIGH on GPIO Out (relay on), above `THRESHOLD_LIGHT` (light) → LOW (relay off),
   in between → change nothing, which protects the relay from chatter.

The extra steps that follow grow it into a motion-activated night light with a page on the home Wi-Fi.

The circuit (GPIO In = GPIO 4, GPIO Out = GPIO 7; +5 V from a breadboard power supply, one common ground):

```
3V3 ── LDR ──┬── GPIO 4 (ADC1)            GPIO 7 ── 10 kΩ (R3) ── B  BC547B
             │                                                    E ── GND
           10 kΩ (R2)                      +5 V ── 10 kΩ (R4) ──┬── C
             │                                                  └── relay module IN
            GND                            relay module VCC = +5 V, GND common

Load, on its own supply: supply + ── COM, NO ── load +, load − ── supply −
```

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [base-twilight-switch](00-base-twilight-switch/) | base | the homework: two thresholds with hysteresis switch the relay; calibrated on the board, no chatter on the logic analyzer |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `include/` + `src/`, and a Wokwi circuit. Until
the lesson is finished, the lesson folder itself is also the working draft (its own `platformio.ini`, `src/`
and `archive/`).

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the PlatformIO
  buttons, or from the terminal: `pio run -d 00-base-twilight-switch`.
- **Simulate:** build the step first, then open its `diagram.json` and press ▶. A potentiometer stands in for the
  light, and the relay module sits straight on GPIO 7 without the transistor, so Wokwi runs the code but not the
  real circuit.
- **Serial:** 115200 baud on the board's UART USB-C port; `monitor_filters = time` puts the wall-clock time on
  every line, which matches the lines to a logic analyzer capture.
- **Capture:** `00` has a Logic 2 capture (`capture.sal`) with GPIO 7 and the decoded UART.
- **Falstad:** `00` also has `falstad.txt`, a model of the circuit's electrics (the link is in its README).
- **Test:** `00` has `wokwi-scenario.yaml` (light → dark → dead zone → light → dead zone, checking GPIO 7);
  run it from the step folder with `wokwi-cli . --scenario wokwi-scenario.yaml` (about 4 s of simulation).
- **Safety:** low voltage only. The load has its own supply, never the board's 5V pin or the breadboard supply.
