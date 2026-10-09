# 05 — Extra: two LEDs on one GPIO

Both LEDs alternate using a single pin.

```
3V3 ── 220 Ω ── blue (+ → −) ──┐
                               ├── GPIO 48
GPIO 48 ── 220 Ω ── red (+ → −) ── GND
```

| Pin state | Red (pin → GND) | Blue (3V3 → pin) |
|---|---|---|
| HIGH (3.3 V) | 3.3 V across → **on** | 0 V across → off |
| LOW (0 V) | 0 V across → off | 3.3 V across → **on** |

**Notes**
- Current flows only where there is a voltage difference. The pin does not "choose" an LED;
  it is either the top or the bottom of each LED's path (push-pull output: source or sink).
- The blue LED is active-low: `LOW` turns it on.
- Use the 3V3 pin, never 5V: the GPIO is not 5 V tolerant.
- With the pin as `INPUT` (high-Z) both LEDs go dark: in series they need ~5 V, and only 3.3 V is available.
  This third state is the basis of Charlieplexing.
