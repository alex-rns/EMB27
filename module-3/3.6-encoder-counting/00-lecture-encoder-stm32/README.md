# 00 — Lecture: a rotary encoder on TIM3's encoder interface

[← Lesson 3.6](../)

The lecture's code (STM32CubeIDE, HAL). CubeMX puts TIM3 in Encoder Mode TI12 on PA6 (TIM3_CH1, A) and PA7
(TIM3_CH2, B). The timer decodes the quadrature itself: every edge of both lines moves its counter up or down (X4),
with no interrupt per edge. The main loop hands the counter to `Encoder_Update()`. Every 100 ms it turns the change
since the last sample into position, direction, angle, RPM (raw and smoothed) and distance. All of them sit in one
`volatile EncoderState encoder` for the debugger's Live Expressions.

```
encoder A ──► PA6 (TIM3_CH1) ─┐
                              ├─► TIM3, encoder mode TI12 (X4) ──► CNT (16 bits, up/down)
encoder B ──► PA7 (TIM3_CH2) ─┘                                       │
                                  main loop: Encoder_Update(&encoder, CNT, HAL_GetTick())
                                                       │ every 100 ms
                     delta, position, direction, angle, RPM (raw, EMA 0.25), distance
```

## Project

| Pin | Set by | Used for |
|---|---|---|
| PA6 | CubeMX: TIM3_CH1, AF2, no pull | encoder A (CLK) |
| PA7 | CubeMX: TIM3_CH2, AF2, no pull | encoder B (DT) |

**CubeMX:** the clock is HSI 16 MHz without the PLL. TIM3 → Combined Channels: Encoder Mode (TI12): prescaler 0,
period 65535, both inputs on the rising polarity with no input filter. STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1,
CubeIDE 2.2.0.

## Code

- `encoder.h`:
  - the constants: `ENCODER_DEFAULT_CPR` 80 (X4 counts per revolution), a 100 ms sample period, the EMA's alpha 0.25
    and a 0.20 m wheel;
  - `EncoderState`.
- `encoder.c`:
  - `Encoder_Init()` clears the state and sets `cpr`.
  - `Encoder_Update()` returns until 100 ms have passed. Then it computes:
    - `delta = (int16_t)(counter - previous counter)`;
    - position += delta, and the direction is the sign of delta;
    - revolutions = position / cpr;
    - RPM = delta × 60000 / (cpr × elapsed ms);
    - angle = revolutions × 360;
    - distance = revolutions × π × D;
    - `rpm_filtered += 0.25 × (rpm - rpm_filtered)`.
- `main.c` (its `USER CODE` blocks): the counter is set to 0, then `Encoder_Init()` and
  `HAL_TIM_Encoder_Start(TIM_CHANNEL_ALL)`. The loop reads CNT and calls `Encoder_Update()` with `HAL_GetTick()`.

**Changed from the files as sent:**
- `encoder.h`: one comment translated (it was in Cyrillic).
- `Encoder_Update()` divides by `state->cpr`. The sent `encoder.c` declared a local `uint32_t cpr` and never set it
  (`-Wmaybe-uninitialized`), so angle, RPM and distance were divided by whatever was on the stack. The teacher fixed
  it the same way in class: no local, `state->cpr` in both formulas (seen on the shared screen).

## Wiring

| Part | Connection |
|---|---|
| KY-040 `CLK` (A) | PA6 |
| KY-040 `DT` (B) | PA7 |
| KY-040 `+` | `3V3`, not 5 V: its pull-ups would put 5 V on PA6/PA7 |
| KY-040 `GND` | `G` |
| KY-040 `SW` | not used |
| SWD header | ST-Link V2 |

## Checked

- Builds with no warnings (CubeIDE's GCC 14.3.1).
- **The encoder module on the logic analyzer** (A/B and SW on three channels, 4 MS/s, 25 s, turned by hand):
  - Between detents both lines rest HIGH.
  - One detent is one full quadrature cycle: 4 edges, 4 counts in X4. So CPR 80 means 20 detents a turn; the detents
    per turn were not counted.
  - Contact bounce is heavy: 1299 raw edges against 159 after a 0.5 ms filter.
  - The X4 transition table (the logic TIM3 applies in hardware) gave the same net count raw and filtered: every
    bounce adds +1 and then -1. A 5 ms filter lost real steps, because fast turns had edges 2–3 ms apart.
- Not run on the board yet.

## Notes

- **The counter is 16 bits.** TIM3's CNT wraps between 65535 and 0. The `(int16_t)` cast of the difference keeps
  delta right across the wrap, as long as fewer than 32768 counts pass between two samples (100 ms).
- **No pull-ups on the chip side.** PA6/PA7 have no pull, and the KY-040's own resistors hold the lines HIGH, so its
  `+` must be wired.
- **No input filter.** Bounce reaches the counter. In X4 a bounce on one line counts +1 then -1 and cancels.
- **The angle is from a software zero:** the counter starts at 0 at reset, and the KY-040 has no index pulse.
- **Which way counts up depends on the wiring:** swap A and B to flip it.
- **On this board PA6/PA7 are also the fitted SPI flash's MISO/MOSI** (its CS on PA4, not driven here). The flash
  drives DO only while it answers a read clocked in on SCK (PA5), so it should stay off PA6. Not tested.
- **The loop has no delay.** `Encoder_Update()` returns early until 100 ms have passed, so the CPU polls all the
  time. The counting itself needs no CPU.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder.
- **Build / debug:** the hammer, then Run → Debug As → STM32 C/C++ Application with the ST-Link connected. Add
  `encoder` to Live Expressions and expand it.
- **CubeMX:** regenerating keeps the `USER CODE` blocks, where the lecture's `main.c` lines live; `encoder.c` and
  `encoder.h` are files CubeMX doesn't touch.
