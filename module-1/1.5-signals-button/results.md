# 1.5 homework: bounce measurements

Capture: Logic 2, 24 MS/s (one sample = 41.7 ns), channel D0 on GPIO 16, 10 presses of different speed and force,
2026-10-14 23:07. Button with a 10 kΩ pull-up to 3V3 plus `INPUT_PULLUP`. The board was not reset: the counter
stood at 170 before the capture and at 206 after it. The Serial Monitor printed each new count with the time it
arrived (`monitor_filters = time`), 29–39 ms after the matching edge on D0, so every line maps to one press or
release.

![The capture in Logic 2: 10 presses on D0](capture-overview.jpg)

Raw data: [`capture.sal`](capture.sal), open it in Logic 2.

## Bounce per press

Bounce pulses = (transitions − 1) / 2. Duration = from the first to the last transition of the press or release.

| # | Press: bounce pulses | Press: bounce, ms | Release: bounce pulses | Release: bounce, ms |
|---|---|---|---|---|
| 1 | 0 | — | 0 | — |
| 2 | 0 | — | 0 | — |
| 3 | 0 | — | 1 | 0.126 |
| 4 | 0 | — | 0 | — |
| 5 | 0 | — | 2 | 0.373 |
| 6 | 0 | — | 0 | — |
| 7 | 0 | — | 2 | 0.065 |
| 8 | 0 | — | 1 | 0.200 |
| 9 | 0 | — | 0 | — |
| 10 | 0 | — | 0 | — |

Total: 6 bounce pulses, all on releases. Shortest pulse 2.87 µs (release 5).

A bouncing release up close (from an earlier capture, 2026-10-13): HIGH, back to LOW for 51 µs, HIGH for 6 µs,
LOW for 126 µs, then HIGH for good.

![A bouncing release in Logic 2](bounce-release-8.png)

## Analyzer vs microcontroller

The counter counts `FALLING` interrupts, so the analyzer column counts HIGH→LOW transitions. Both columns are split
into the press and its release.

| # | Analyzer: HIGH→LOW (press + release) | MCU: counter increments (press + release) |
|---|---|---|
| 1 | 1 + 0 | 1 + 1 |
| 2 | 1 + 0 | 1 + 1 |
| 3 | 1 + 1 | 1 + 4 |
| 4 | 1 + 0 | 1 + 1 |
| 5 | 1 + 2 | 1 + 6 |
| 6 | 1 + 0 | 1 + 1 |
| 7 | 1 + 2 | 1 + 5 |
| 8 | 1 + 1 | 1 + 4 |
| 9 | 1 + 0 | 1 + 1 |
| 10 | 1 + 0 | 2 + 1 |
| **Total** | **16** (10 + 6) | **36** (11 + 25) |
