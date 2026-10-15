# 06 — Extra: buttons on interrupts

The assignment says the mode must change "after a single press". In [05](../05-base-combined/) the buttons are
read only between the `delay()` calls, so a short press during a pause is lost. Here each button has an
interrupt: the chip calls a short function the moment the pin falls from HIGH to LOW (`FALLING`), even in the
middle of `delay()`, and that function only sets `mode`. `loop()` just blinks according to `mode`.

| Part | GPIO |
|---|---|
| Green LED (through 220 Ω, cathode → GND) | 15 |
| Blue LED (through 220 Ω, cathode → GND) | 16 |
| External button (other leg → GND), `INPUT_PULLUP`, interrupt on `FALLING` | 21 |
| BOOT button (on the board), `INPUT`, interrupt on `FALLING` | 0 |

![Circuit in Wokwi](diagram.png)

[Demo on the real board (Google Photos)](https://photos.app.goo.gl/mzhqbWubczvtSgcn8)

**Notes**
- Edge instead of level: polling asks "is it pressed now?", the interrupt reacts to "it was just pressed".
  The press is never lost, but the new mode starts when the current `if` branch ends: up to 2 s in mode 2,
  up to 0.4 s in mode 1.
- `mode` is written in an interrupt and read in `loop()`, so it is `volatile`: the compiler doesn't know the
  interrupt exists and could otherwise keep a stale copy in a register (e.g. in `while (mode == 1) {}`).
- `IRAM_ATTR` puts the handler in internal RAM. Code normally runs from flash through a cache, which is off
  while the flash is being written. In Arduino core 2.0.17 `attachInterrupt` doesn't register the interrupt
  as IRAM-safe, so it is held back while the cache is off, and a handler in flash works too (lesson 1.5 does
  that). A handler left in flash would crash the board only if its interrupt were registered with the IRAM flag.
- Contact bounce calls a handler several times per press. That is harmless here because each call sets the
  same value; a handler that toggles or counts would need debouncing.
- Handlers stay short: no `delay()`, no `Serial.print()`.
- The circuit is the same as in [03](../03-base-mode1-sync/). Test BOOT on the real board with the Serial
  Monitor closed or with the cable in the native USB port.
