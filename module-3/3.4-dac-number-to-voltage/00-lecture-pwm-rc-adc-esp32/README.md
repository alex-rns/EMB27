# 00 — Lecture: PWM through an RC low-pass as a DAC, read back by the ADC (ESP32-S3)

[← Lesson 3.4](../)

The ESP32-S3 has no DAC, so the lecture builds one from PWM: LEDC puts out 20 kHz on GPIO 18, a 10 kΩ / 100 nF
low-pass turns it into its average voltage, and ADC1 reads that voltage back on GPIO 1. Every 20 ms the code sets a
new duty that follows a sawtooth, a sine or a triangle (one shape for 3 periods of 4 s, then the next) and prints the
measured voltage as `voltage:1.234`; `plot.py` draws those lines live. The teacher's code (ESP-IDF in C, PlatformIO,
ESP-IDF 6.1); only its comments were translated.

```
LEDC 20 kHz, 10 bits ──► GPIO 18 ──[10 kΩ]──┬──► GPIO 1 ──► ADC1 (16 readings averaged, calibrated) ──► "voltage:…"
duty = 20..80 % of the wave              [100 nF]
                                           GND
```

## Code (`src/main.c`)

- `init_pwm()`: LEDC timer 0, low-speed, 10 bits (1024 steps), 20 kHz; channel 0 on GPIO 18, duty 0.
- `init_adc()`: ADC1 oneshot, channel 0 (GPIO 1), 12 dB, 12 bits, with the curve-fitting calibration.
- `wave_value(waveform, phase)`: the shape from 0 to 1 for a phase from 0 to 1: sawtooth `phase`, sine
  `0.5 - 0.5 cos(2π phase)`, triangle `1 - |2 phase - 1|`.
- `read_voltage_mv()`: 16 conversions averaged, then the calibration's millivolts.
- `app_main()`: every 20 ms (`vTaskDelayUntil`, so the period doesn't drift with the loop's own time) the duty
  becomes `(0.2 + 0.6 × value) × 1024`, i.e. 20..80 %, about 0.66..2.64 V after the filter; then the voltage is
  printed. After 3 periods (600 samples) the next shape starts. `setvbuf(stdout, NULL, _IONBF, 0)` makes each line go
  out at once.

## Wiring

| Part | Connection |
|---|---|
| Resistor 10 kΩ | GPIO 18 → GPIO 1 |
| Capacitor 100 nF (ceramic, "104") | GPIO 1 → `GND` |
| Multimeter (optional) | `+` to GPIO 1, `-` to `GND` |
| USB | the board's UART port (the CH343P bridge) |

The teacher's code comment draws 22 µF; the kit has 10 nF, 100 nF and 100 µF, and 100 nF was used here.

## Checked on the board

14 s of output (688 lines): the sawtooth rises 0.63 → 0.73 → 0.83 → … → 2.50 → 2.59 V in 200 steps of 20 ms and drops
back to 0.63 V within one step; then the sine (0.66 → 0.88 → 1.28 → 1.78 → 2.23 → 2.52 …). Over three periods: minimum
0.632 V, maximum 2.596 V, against about 0.66 and 2.64 V expected (3.3 V × 20 % and × 80 %). `plot.py` showed the same
shapes live.

## Notes

- **The filter's time constant has to sit between the PWM period and the wave's step.** 10 kΩ × 100 nF = 1 ms: the
  cutoff 1 / (2π RC) is about 160 Hz, far below 20 kHz, so the PWM is smoothed (ripple roughly V × D(1 - D) /
  (f × RC), about 40 mV at 50 %: an estimate, not measured), and far above the 20 ms steps, so the sawtooth's drop
  shows in one step. The teacher's 22 µF (RC = 0.22 s) smooths more and rounds the shapes; the kit's 100 µF (1 s)
  would flatten them a lot.
- **The readings sit 30–40 mV below 3.3 V × duty:** the pin's high level is a little under 3.3 V and the ADC has its
  own error (not separated here).
- **One program per serial port.** While `plot.py` (or a serial monitor) has the port open, flashing fails: close it
  first.
- **The S3 has no DAC at all** (`driver/dac.h` doesn't exist for it); the classic ESP32's two 8-bit DACs on GPIO 25/26
  are what the lesson's text uses.

## Working with the step

- **Build / Upload:** `pio run -d 00-lecture-pwm-rc-adc-esp32` (add `-t upload`), or the PlatformIO buttons with this
  folder in the VS Code workspace.
- **Plot:** once, `python3 -m venv .venv && .venv/bin/python -m pip install -r requirements.txt`; then
  `.venv/bin/python plot.py --port /dev/cu.usbmodem…` (the board's UART port; `COM5`-style names on Windows).
