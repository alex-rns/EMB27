# 00 — Lecture: a light sensor switches an LED

An LDR in a voltage divider on GPIO 4 (ADC1) and an LED on GPIO 7. Once a second the firmware reads the ADC,
smooths the reading with a moving average of the last 8 samples and switches the LED with hysteresis: on below
1600, off above 2200. It prints `raw:<n>  avg:<n>  mV:<n>  led:<0|1>` with labels a serial plotter understands.

| File | What it does |
|---|---|
| `include/config.h` | pins, period, window size and thresholds; `static_assert` checks them at compile time |
| `src/sensor.cpp` | 12-bit ADC at 11 dB; `analogRead()` plus a separate `analogReadMilliVolts()` conversion; the moving average in a ring buffer |
| `src/smart_light.cpp` | the LED with hysteresis |
| `src/main.cpp` | a non-blocking 1 s period with `millis()`, then the print |
| `src/utils/utils.h` | a `WATCH(x)` debug macro, not used |

| Part | GPIO |
|---|---|
| LDR (other leg → 3V3) and 10 kΩ (other end → GND), their midpoint | 4 |
| LED anode through 220 Ω (cathode → GND) | 7 |

Bright light lowers the LDR's resistance, so the midpoint moves towards 3.3 V and the ADC code rises.

![Circuit in Wokwi](diagram.png)

**Wokwi** has no bare LDR, only a photoresistor module, and the module's divider is the other way round
(10 kΩ to VCC, LDR to GND): more light gives a lower voltage. In the simulator the LED therefore lights up in the
light and goes off in the dark. The simulated ADC is ideal (3.3 V = 4095), unlike the real one.

## Measured on the board

Serial Monitor, room light: `raw` 2598–2607 (±5 codes of noise), `avg` steady at 2601–2602.
`raw / 4095 × 3.3` gives 2.10 V while `analogReadMilliVolts()` gives 2.19 V, about 4 % apart; a multimeter on
GPIO 4 decides which one is closer.

[`capture.sal`](capture.sal), Logic 2, 24 MS/s, 7.9 s: D0 = GPIO 4 (the analyzer turns the midpoint into one bit
through its fixed threshold), D1 = GPIO 7 (LED), D2 = TX (GPIO 43), decoded as Async Serial at 115200 baud.
The LDR was covered by hand from 1.2 s to 7.0 s.

| Time, s | raw | avg | led |
|---|---|---|---|
| 0.565 | 2637 | 2696 | 0 |
| 1.565 | 1211 | 2510 | 0 |
| 2.565 | 1286 | 2345 | 0 |
| 3.565 | 1159 | 2143 | 0 |
| 4.565 | 1555 | 2013 | 0 |
| 5.565 | 1127 | 1810 | 0 |
| 6.565 | 1239 | 1619 | 0 |
| 7.565 | 2471 | 1585 | 1 |

- The LED switched on 6.4 s after the LDR was covered and 0.55 s after it was uncovered: each new sample
  replaces one of eight, so `avg` moved only about 180 codes a second.
- The light came back from 3.74 to 4.26 s. The samples at 3.565 and 4.565 s both fell outside that half
  second, so the firmware never saw it: sampling once a second misses anything shorter than a second.
- The order of `loop()` in time: the ADC sample at 7.564980 s (a dip on D0, below), the LED 131 µs later,
  the printed line 55 µs after the LED.
- A 38-character line takes 3.30 ms on the wire (10 bits per character at 115200 baud). Lines come every
  1000.047 ms: the board's and the analyzer's crystals differ by about 47 ppm.
- D0 is far from a clean bit: 29 246 edges. While the hand moves, the voltage crosses the analyzer's threshold
  slowly and noise flips it back and forth, 8083 edges in 50 ms. That is why digital inputs have a Schmitt
  trigger, and it is what the two thresholds in `config.h` do in software.
- Once per loop, about 185 µs before the line, D0 dips for 125–167 ns even in bright light, most likely when
  the ADC connects its sampling capacitor to the pin. While the LDR is covered, a 0.75 µs spike comes every
  20.0005 ms; its source is not identified.

**Notes**
- Use ADC1 pins (GPIO 1–10): ADC2 is shared with Wi-Fi.
- At 11 dB the ESP32-S3 ADC measures up to about 3.1 V, not 3.3 V; anything above reads 4095.
  `analogReadMilliVolts()` applies the chip's calibration, `analogRead()` returns the raw code.
- The thresholds are demo values: set them from readings on your own breadboard and in your own light.
- A 100 nF capacitor from the ADC pin to GND is the usual cure for the noise and the dips above (not tried yet).
