# 00 — Lecture: debounce on a logic analyzer

A button on GPIO 4 with an external 10 kΩ pull-up (`INPUT`, no internal pull-up) is watched by an interrupt on
`CHANGE`; comment out `USE_INTERRUPT_DEBOUNCE` to poll it in `loop()` instead. The firmware copies three
signals to pins, so the logic analyzer shows them next to the button itself:

- GPIO 5 (RAW): the level the firmware last saw;
- GPIO 6 (CLEAN): the debounced level, accepted after 30 ms with no button activity;
- GPIO 7: a reference square wave, 400 ms period, 100 ms HIGH.

Once a second it prints `RAW=<n> | DEBOUNCED=<n>`: presses counted with bounce, and after debouncing.

| Part | GPIO |
|---|---|
| Button (other leg → GND) + 10 kΩ pull-up (other end → 3V3) | 4 |
| Logic analyzer D0 (channel 1 on the case) | 4 |
| Logic analyzer D1 (channel 2) | 5 |
| Logic analyzer D2 (channel 3) | 6 |
| Logic analyzer D3 (channel 4) | 7 |
| Logic analyzer GND | GND |

![Circuit in Wokwi](diagram.png)

**Measured on the board**

[`capture.sr`](capture.sr): PulseView 0.4.2, 24 MHz (one sample = 41.7 ns), 9.06 s. Open it in PulseView.

![The capture in PulseView](pulseview.jpg)

The shaded part (1438–1653 ms) is the press with a 14.9 ms gap: D0 and D1 jump HIGH for a moment,
D2 stays LOW. D2 follows D0 about 30 ms late all along; D3 is the reference square wave.

| Channel | Signal | Result |
|---|---|---|
| D0 | button | 32 HIGH→LOW transitions; bounce only on releases |
| D1 | RAW | follows D0 6.5–12.8 µs later (median 8 µs); 30 HIGH→LOW transitions |
| D2 | CLEAN | 23 presses; switches 29–30 ms after the last edge on D0 |
| D3 | reference | period 400.016–400.020 ms |

Bounce on D0 and the same moments on D1:

| Time, s | D0: transitions | D0: shortest pulse | D1: transitions |
|---|---|---|---|
| 4.535 | 5 | 0.21 µs | 3 |
| 6.711 | 3 | 70 µs | 3 |
| 7.317 | 5 | 9.6 µs | 5 |
| 7.745 | 3 | 26.8 µs | 3 |
| 7.950 | 3 | 0.96 µs | 1 |
| 8.543 | 3 | 0.63 µs | 1 |

D0 also stayed HIGH for only 14.9 ms during one press and for 24.2 ms between two presses. Both are shorter
than the 30 ms window, so D2 treated them as bounce and merged them.

**Notes**
- `INPUT` turns the internal pull-up off: without the external 10 kΩ the pin floats and reads random levels.
  Wokwi doesn't show that: its floating pin reads 0.
- The analyzer's GND must be connected to the board's GND, or it has no reference to measure against.
- The case labels the analyzer channels 1–8; Logic 2 and PulseView call them D0–D7.
- A debounce window also swallows real presses or releases shorter than the window (the 24.2 ms gap above).
- The board and the analyzer each count time with their own crystal. They disagree by about 40 ppm, which
  is why the 400 ms reference measures as 400.016 ms.
- Printing once a second shifts the timing of D1 and D3 slightly (the code says so).
- Logic 2 and PulseView can't share the analyzer: each loads its own firmware into it. Close one and replug
  the analyzer before using the other.
