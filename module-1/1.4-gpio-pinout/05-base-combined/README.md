# 05 — Base: both modes, latched

The full homework. Mode 1 runs at start. The external button selects mode 1 (both LEDs together, 200 ms),
BOOT selects mode 2 (the LEDs in turn, 1000 ms), and the chosen mode stays after the button is released,
until the other button is pressed. Built from [03](../03-base-mode1-sync/) and [04](../04-base-mode2-alternate/).

| Part | GPIO |
|---|---|
| Green LED (through 220 Ω, cathode → GND) | 15 |
| Blue LED (through 220 Ω, cathode → GND) | 16 |
| External button (other leg → GND), `INPUT_PULLUP` | 21 |
| BOOT button (on the board), `INPUT` | 0 |

![Circuit in Wokwi](diagram.png)

**Notes**
- The latch is the global `mode`: the buttons only change it, and `loop()` blinks according to it.
  A local variable would be created anew on every `loop()` call and forget the mode.
- The buttons are read once per cycle, at the top of `loop()`; the rest of the time the program sits in
  `delay()`. A press has to last until the cycle ends: up to 0.4 s in mode 1, up to 2 s in mode 2.
  Non-blocking timing with `millis()` comes in module 2.
- The circuit is the same as in [03](../03-base-mode1-sync/). Wokwi's DevKitC-1 model has no pull-up on GPIO 0,
  so in the simulation GPIO 0 reads LOW and the program stays in mode 2. On the real board it works.
  Adding a 10 kΩ from GPIO 0 to 3V3 to `diagram.json` makes the simulation match the board.
- GPIO 0 is a strapping pin: holding BOOT during a reset or power-on starts the download mode.
- Test BOOT with the Serial Monitor closed or with the cable in the native USB port: a monitor open on the
  UART port can hold GPIO 0 LOW, and the program acts as if BOOT were pressed.
