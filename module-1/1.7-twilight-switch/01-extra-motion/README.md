# 01 — Extra: a motion-activated night light (a PIR sensor, a 15 s timeout)

The twilight switch from step 00 grows into a night light: the lamp now switches on only when it is **dark and
something moves**, and goes off 15 s after the last motion. An HC-SR501 PIR sensor on GPIO 5 sees the motion, the LDR
on GPIO 4 and the relay stage on GPIO 7 stay as in step 00.

The rules (`Lamp::update()` is the only code that writes GPIO 7):

- lamp off, dark and PIR OUT HIGH → lamp on (`lamp on (dark + motion)`);
- lamp on and 15 s (`MOTION_TIMEOUT_MS`) since the **falling edge** of PIR OUT → lamp off (`lamp off (no motion 15 s)`);
- every new motion while the lamp is on restarts the 15 s;
- light never switches the lamp off, it only prevents switching on.

Once a second the firmware prints a status line, and one more line per lamp event (the time in brackets is the
uptime). Real board output, the Serial lines decoded from channel D2 of [`capture.sal`](capture.sal) (the
left number is the time in the capture, in seconds; the middle of the countdown is elided):

```
  0.0001  [  977.8 s] lamp on (dark + motion)
  0.1738  raw: 681 | dark | motion: now | lamp: on, 15 s left
  1.1738  raw: 719 | dark | motion: 0.2 s ago | lamp: on, 15 s left
  3.1739  raw: 681 | dark | motion: 2.2 s ago | lamp: on, 13 s left
  4.1739  raw: 692 | dark | motion: now | lamp: on, 15 s left      <- the second PIR pulse restarted the countdown
  5.1740  raw: 714 | dark | motion: 0.0 s ago | lamp: on, 15 s left
  ...
 19.1747  raw: 695 | dark | motion: 14.0 s ago | lamp: on, 1 s left
 20.1556  [  997.9 s] lamp off (no motion 15 s)
 20.1747  raw: 727 | dark | motion: 15.0 s ago | lamp: off
```

| File | What it does |
|---|---|
| `include/config.h` | pins (`PIR_GPIO` = 5), `MOTION_TIMEOUT_MS` = 15000, thresholds, `RELAY_ON_IS_HIGH` |
| `src/motion.cpp` | reads PIR OUT every loop with `INPUT_PULLDOWN`; remembers the last loop that saw HIGH, i.e. the falling edge |
| `src/lamp.cpp` | the auto rule above; `secondsLeft()` for the status line |
| `src/light_sensor.cpp`, `src/main.cpp` | as in step 00, plus the motion and lamp fields in the status line |

| Part | GPIO |
|---|---|
| LDR (other leg → 3V3) and 10 kΩ (other end → GND), their midpoint | 4 |
| BC547B base, through R3 = 10 kΩ ("GPIO Out") | 7 |
| HC-SR501 OUT (VCC on the breadboard supply's +5 V rail, GND common) | 5 |

The circuit is step 00's, plus the PIR:

```
3V3 ── LDR ──┬── GPIO 4 (ADC1)            GPIO 7 ── 10 kΩ (R3) ── B  BC547B
             │                                                    E ── GND
           10 kΩ (R2)                      +5 V ── 10 kΩ (R4) ──┬── C
             │                                                  └── relay module IN
            GND                            relay module VCC = +5 V, GND common

HC-SR501:  VCC ── +5 V (breadboard supply rail)     OUT ── GPIO 5     GND ── GND (common with the board)

Load, on its own supply: supply + ── COM, NO ── load +, load − ── supply −
```

![Schematic: step 00's circuit plus the HC-SR501 PIR on GPIO 5](schematic.svg)

![Circuit in Wokwi](diagram.png)

**Wokwi** has the same stand-ins as step 00 (a potentiometer for the light, the relay module straight on GPIO 7 with
an LED on NO), plus a `wokwi-pir-motion-sensor` (OUT → GPIO 5, VCC on the board's 5V pin, hold time set to 3 s;
Wokwi has no shorter one). The PIR part can't be triggered from a scenario or from the Wokwi MCP, only by clicking it
in the VS Code editor: in the dark, Simulate motion switched the lamp LED on, and it went off after the hold plus
15 s. [`wokwi-scenario.yaml`](wokwi-scenario.yaml) tests what can be tested without motion: potentiometer to dark →
`dark | motion: never | lamp: off` with GPIO 7 LOW (darkness alone never switches the lamp on), potentiometer to
light → `light | motion: never | lamp: off`. It runs locally only (a few seconds of simulation). In Wokwi GPIO 7 reads
HIGH for the first 76.6 ms after reset (undriven until `Lamp::begin()`), so a test must not look at it before then.

## The PIR module

An HC-SR501 clone (AliExpress starter kit), the PCB marked **HW-416-B**.

- **Pins.** Labelled VCC / OUT / GND on the lens-side PCB edge (lens towards you, pins pointing away: VCC left,
  OUT middle, GND right). There are no labels on the back.
- **Power.** VCC from the breadboard power supply's +5 V rail (MB-102 type, fed over USB), GND common with the
  board. The board's own 5V pin carries no USB power on this clone (see step 00).
- **OUT level.** Measured with a multimeter (black probe on GND): idle 0.00 V, motion 3.34 V. Never 5 V, so OUT
  can go straight to a GPIO.
- **Trimmers.** Two unlabelled 1 MΩ trimmers (marked "105"), both fully counter-clockwise: the minimum hold time
  (about 1.0 s here) and the minimum range. The firmware alone should own the timeout, and at a desk a long range
  retriggers on every small move.
- **Trigger mode.** Set by three solder pads marked H / L, no jumper. Measured to be **H** (repeatable): see below.

## Measured on the board

**Setup check, before OUT was connected.** Multimeter on OUT: 0 V idle, 3.34 V while a hand waves. Only then OUT
went to GPIO 5.

**H or L?** Logic 2, D1 = GPIO 5, trigger on a D1 rising edge. In L (single trigger) every
pulse would be as wide as the hold time. With about 10 s of deliberate waving (a separate capture, not saved) the widths were not equal:

| Pulse on OUT | Width |
|---|---|
| first | 2.547 s |
| second | 4.752 s |
| a single detection (hold at minimum) | ≈ 1.0 s |

Pulses 2.5 s and 4.75 s wide are far above the ≈ 1.0 s of one detection: each new detection extended the pulse, so
the module is in H. Between the pulses OUT stays LOW for about 2.5–2.8 s after every falling edge (a 2.641 s gap
between the two above) whatever happens in front of the sensor: the module's blocking time. One wave often gives
two pulses (see the capture below). Waving across the module's view is not the only way to trigger it: a person
moving at the desk was enough even at minimum range.

**The timeout.** [`capture.sal`](capture.sal), Logic 2, 4 MS/s, 40 s, trigger on D1 rising: D0 = GPIO 7 (lamp
relay), D1 = GPIO 5 (PIR OUT), D2 = TX (GPIO 43), in the dark. One wave:

| Edge | Time, s | Result |
|---|---|---|
| D1 ↑ (first pulse) | 0.0000000 | |
| D0 ↑ (lamp on) | 0.0000035 | reaction 3.5 µs (second cycle 4.8 µs); the `lamp on (dark + motion)` line follows 0.1 ms later |
| D1 ↓ (first pulse) | 0.949 | |
| D1 ↑ ↓ (second pulse, same wave) | 3.678 – 5.156 | gap 2.73 s = the blocking time |
| D1 ↓ (last) | 5.1556880 | |
| D0 ↓ (lamp off) | 20.1555788 | **14.99989 s** after the last D1 fall (planned 15.0 ± 0.05) |

Measured from the first pulse's fall the timeout would be 19.21 s: the countdown restarted on the second pulse
(Serial: `13 s left` → `15 s left`). Exactly one on/off pair on D0. A later D1 rise at 33.49 s (someone moving
after the click) started a new cycle.

**The restart.** A second capture (not saved), same setup, a second wave 7.70 s into the countdown (Serial: `9 s
left` → `15 s left`). Wave 1 gave D1 high 0–1.676 s, wave 2 gave two pulses (9.380–10.329 s and 13.163–14.692 s),
and D0 fell 15.00009 s after the last D1 fall, 28.0 s after wave 1's fall. Reaction time 4.5 µs. The first wave's
countdown never ended.

**A loose OUT wire.** OUT and the analyzer's D1 probe both off GPIO 5, the LDR covered (dark): 15 status lines over
about 15 s, `dark | motion: 85.4 → 99.4 s ago | lamp: off`, no event line. What is known for sure is that pasted
15 s log and that no motion was seen for 99.4 s before its last line; how long the wire was actually out wasn't
recorded, and the planned 60 s test was not run. `INPUT_PULLDOWN` holds the open pin LOW: no phantom motion and
the lamp stays off. Without the pull-down an open input floats and can read HIGH at random.

**Not run on the board.**
- *The lamp's own light on the LDR.* The LED strip hadn't arrived, so this stayed open (the open item from step
  00). By construction light can't switch the lamp off: `Lamp::update()` switches it off only on the timeout. Wokwi
  showed it too: pot to light while the lamp was on → the countdown went on. The real placement check waits for the
  strip.
- *A passing shadow.* A hand passing over the LDR in room light was not run on the board. Step 00 measured a hand
  about 10 cm above the LDR at raw 2317–2343, at least 700 codes above `THRESHOLD_DARK` = 1600. Only a direct
  cover (a finger, raw ≈ 1000) goes below it, so a realistic shadow can't make the sensor read dark. If a close
  shadow plus motion ever switches the lamp on, the fix is the 8-sample moving average (0.8 s) from the lesson 1.6
  code.

**Notes**
- **Warm-up.** HC-SR501 datasheets give a 30–60 s start-up during which OUT can toggle without motion. Not
  watched or measured here (the board was already running when the PIR was added), so don't trust the first minute after power-up:
  a false trigger in the dark would switch the lamp on for 15 s.
- **The PIR's own hold adds to the 15 s.** The countdown starts at the falling edge of OUT, and OUT stays HIGH for
  the module's hold time after the last detection (about 1.0 s at minimum). So the lamp is on about 16 s after the
  last real motion, and every wave, however short, costs at least the hold time. Turning the hold trimmer up adds
  that much to every timeout.
- **Why light only blocks switching on.** If the lamp also switched off in the light, its own light on the LDR
  would trigger the loop: motion → lamp on → the strip lights the LDR → "light" → lamp off → dark again → lamp on
  at the next motion. The timeout alone decides when to switch off. Keep the LDR pointed away from the strip anyway.
- **Power order with two supplies.** The PIR runs from the breadboard supply, the ESP32 from USB. If the supply is
  on and the ESP32 is not, OUT (3.3 V) drives an unpowered ESP32 pin, which is out of spec. Power the ESP32 first,
  or switch both together.
- **A person sitting still is invisible.** A PIR reacts to changes in infrared, not to presence. Someone sitting
  still in the dark lets the lamp go off after 15 s.
- **The range doesn't stop at minimum.** At minimum range the PIR still saw a person at the desk, so a night light
  on a desk needs the sensor aimed away from where people sit.
- **A probe changes the pin.** A logic analyzer on GPIO 5 loads the line, and on GPIO 7 it clicked the relay at every
  boot in step 00. D1 was taken off for the loose-wire test, so the test showed the bare pin and not the analyzer's
  pull.
- **The 1 s tick of the status line hides edges.** The line shows `motion: now` or the tenths since the last HIGH,
  truncated, not rounded. Use the analyzer for timing, not the Serial lines.
