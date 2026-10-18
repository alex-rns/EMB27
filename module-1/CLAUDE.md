# Module 1 — technical notes (measurements, circuits, basic electronics)

> Claude Code loads this file when working inside `module-1/`. Project rules live in `../CLAUDE.md`.
> Update after every lesson of this module.

## Lessons → folders → status

| Lesson | Topic | Folder | Status |
|---|---|---|---|
| 1.1 | Measuring current and voltage | — | lecture only |
| 1.2 | Can you burn an LED? Resistance and voltage | — | lecture only |
| 1.3 | ESP32: first program, Blink | `1.3-esp32-blink` | ✅ finished (base + 4 extras) |
| 1.4 | GPIO and pinout, reading the datasheet | `1.4-gpio-pinout` | ✅ finished: `00`, `01` (lecture), `03`–`05` (homework), `06` (extra: interrupts); `02` unused |
| 1.5 | Signals and a button (oscilloscope) | `1.5-signals-button` | ✅ finished: `00` (lecture: debounce), `01` (homework: interrupt counter vs analyzer) |
| 1.6 | Intro to the ADC: reading light | `1.6-adc-light` | ✅ finished: `00` (lecture: smart light), `01` (homework: formula vs `analogReadMilliVolts`) |
| 1.7 | Mini project: light sensor controls an LED | `1.7-twilight-switch` | ✅ saved 00, 01 · ⏳ /finish-lesson |
| 1.8 | Debrief: typical mistakes (LED, polarity, wiring) | — | — |
| 1.9 | Basic soldering | — | — |

Module 1 has no flexible slot (those are in modules 2–6).

## Hardware in this module

- ESP32-S3 DevKitC-1 clone, N16R8 (see the root CLAUDE.md). Which GPIOs drive the onboard LEDs (47, 48) is still
  to verify: 1.3 uses both for external LEDs.
- LEDs (red, blue, white, green), 220 Ω and 10 kΩ resistors, tactile button, breadboard, LDR (1.6).
  Coming: multimeter checks, soldering iron (1.9).
- Logic analyzer (1.5): 8-channel 24 MHz Saleae Logic clone, digital only, fixed input threshold. The case labels
  the channels 1–8, Logic 2 and PulseView call them D0–D7. Its GND goes to the board's GND.
- Pins used so far: 1.3: 47, 48, 21. 1.4: 45 (lecture, strapping pin on purpose), 15, 16, 21, 0 (homework).
  1.5: 4, 5, 6, 7 (lecture), 16 (homework). 1.6: 4 (LDR, ADC1), 7 (lecture LED). 1.7: 4 (LDR, ADC1), 5 (PIR OUT), 7 (GPIO Out → BC547B).
- GPIO 0 (BOOT) has an external pull-up on this board; Wokwi's DevKitC-1 model doesn't, so a Wokwi circuit
  that reads BOOT with `INPUT` needs a 10 kΩ from GPIO 0 to 3V3 in `diagram.json`.
- Avoid: 19/20 (native USB — the board drops off the port), 0/3/45/46 (strapping),
  26–37 (flash/PSRAM), 43/44 (UART0).

## What to watch for (common mistakes, including in AI-generated code)

- **GPIO:** input vs output, pull-up/pull-down, push-pull vs open-drain.
  Watch for a pin mode that was never set, a forgotten pull-up.
- **Button (1.5):** a button read as plain `INPUT` with no pull resistor gives a floating pin,
  which reads random values. Contacts bounce on releases too, so a `FALLING` counter counts releases;
  a debounce must filter both directions.
- **ADC (1.6–1.7):** ESP32 `analogRead()` is 12-bit (0–4095), not 10-bit (0–1023) like an Arduino Uno;
  generated code often assumes 1023. At 11 dB the ESP32-S3 measures up to ~3.1 V, not 3.3 V ("3.3 V = 4095" is
  wrong); it is non-linear near the rails. `analogRead()` is the raw code, `analogReadMilliVolts()` is calibrated.
  Check which ADC unit a pin belongs to: prefer ADC1 (GPIO 1–10), because ADC2 is shared with Wi-Fi.
  Watch for `raw / 4095` in integers (always 0) and for a negative difference stored in an unsigned type.
- **PIR (1.7 step 01):** HC-SR501 clone (HW-416-B), pins VCC / OUT / GND on the lens-side edge; VCC from the
  breadboard supply's 5 V, OUT is 3.3 V (measured 3.34 V) into GPIO 5. H/L solder pads: this module is in H
  (repeatable; measured by pulse widths, not by the label). Hold ≈ 1.0 s and a ≈ 2.5–2.8 s blocked gap after every
  fall at the minimum trimmers; the PIR hold adds to the firmware's 15 s. Read OUT with `INPUT_PULLDOWN` so a loose
  wire reads "no motion". A person at the desk triggers it even at minimum range.
- **LEDs:** polarity, a resistor from Ohm's law, max current per pin (datasheet).
- **Datasheet:** pin functions, strapping pins, electrical characteristics.

## Lesson notes

**1.3 — ESP32 Blink, homework "police light".**
The preprocessor cuts code under `#if`, so all flags at `0u` give an empty `loop()`; the cut code never
reaches flash (a runtime `if` keeps both branches).
`pinMode` after `ledcAttachPin` detaches PWM from the pin.
220 Ω at 3.3 V: red ≈ 5.9 mA, blue ≈ 1.4 mA. Blue and white are dimmer because they get less current
(Vf ≈ 3 V leaves ~0.3 V across the resistor), not because they need more. With so little headroom the
Vf spread between parts (≈ 2.8–3.2 V) changes the current a lot. Break-before-make.
Variable speed: `#define` → variable, global because `loop()` runs again and again;
`int`, not `uint32_t`, because an underflow would turn into a 49-day `delay`.
Two LEDs on one pin: 3V3→LED→pin (active-low) + pin→LED→GND — source/sink, push-pull.
`INPUT` is high-Z: the pin neither sources nor sinks current, it only senses the voltage. Step 05 then
goes dark because the only path left (3V3 → blue → red → GND) needs ~5 V.
Current flows only where there is a voltage difference; through a diode only from + to −.
Two push-pull outputs driving opposite levels short each other (hence open-drain in I2C).
Steps: `1.3-esp32-blink/00…05` — lecture demo, base, 4 extras; each an ordinary PlatformIO project.
Steps 01–03 were reconstructed afterwards (they were only intermediate edits of one `main.cpp`).

**1.4 — GPIO and pinout, homework "two LEDs, a button and BOOT".**
Pull-down: resistor to GND, button to 3V3, pressed = 1. Pull-up: resistor to 3V3, button to GND, pressed = 0.
The resistor sets the idle level, the button pulls the pin to the other rail; a button to the same rail
changes nothing. `INPUT_PULLUP`/`INPUT_PULLDOWN` switch on the chip's ~45 kΩ resistor; plain `INPUT` switches
it off (Arduino 2.0.17 `pinMode`), so the pin floats unless something on the board holds it.
GPIO 0 (BOOT) has an external pull-up on this board, so `INPUT` works; Wokwi's DevKitC-1 model lacks it and
reads GPIO 0 as LOW. Wokwi also never shows a floating pin's noise: it reads 0.
A Serial Monitor open on the UART port (CH343P) can hold GPIO 0 LOW through the DTR/RTS auto-download
transistors; a program reading BOOT then acts as if it were held.
Strapping pins: GPIO 0 picks the boot mode, GPIO 45 the flash voltage. Holding them at the wrong level
during a reset can keep the board from booting; an external pull-up on 45 does that at every reset.
Breadboard: a tactile button across the gap joins its rows only when pressed; legs in one row are always
connected. A pin in the 3V3 row reads 1 all the time.
`Serial` is UART0 (the UART USB-C port); `monitor_speed` must match `Serial.begin()` or the monitor shows garbage.
`delay()` blocks: buttons read once per cycle miss short presses (up to 0.4 s / 2 s). Interrupts on `FALLING`
react to the edge, not the level: the press is latched in hardware even during `delay()`. Handlers are
short (no `delay`, no `Serial`) and share data through `volatile` variables. `IRAM_ATTR` is optional in core
2.0.17: `attachInterrupt` doesn't register IRAM-safe interrupts, so they wait while the flash is busy (1.5).
Bounce calls a handler several times; harmless while it sets a value, not when it toggles.
Steps: `1.4-gpio-pinout/00, 01` lecture (pull-down, pull-up), `03–05` homework in stages, `06` extra (interrupts).

**1.5 — Signals and a button, homework "button bounce on a logic analyzer".**
The analyzer records only HIGH/LOW, at 24 MS/s (41.7 ns per sample); no analog shape, no rise time.
Logic 2 and PulseView each load their own firmware into it: close one and replug before using the other.
PulseView starts at 20 kHz after a restart, too coarse for bounce: check the rate before every run.
Bounce of the kit's tactile button, measured: on releases only, 0.07–0.4 ms in the homework, up to ~1.2 ms
seen earlier, pulses down to 0.2 µs. Not the 5–20 ms the lesson text gives.
`FALLING` counter vs analyzer over 10 presses: 16 falling edges, 36 counts. The counter over-counts on releases:
+1 even on a release the analyzer saw as clean, +4…+6 on a bouncy one. Cause not verified: the chip and the
analyzer have different input thresholds, and the analyzer can't see a pulse shorter than one sample.
The firmware also misses pulses: under ~1 µs they vanish, from ~10 µs on it sees them (lecture capture).
The interrupt runs microseconds after the edge and reads the pin; two edges before it runs leave one pending flag.
Lecture debounce: the `CHANGE` ISR only records (level, time, count), `loop()` decides. The quiet window
restarts on every edge in both directions; a level is accepted after 30 ms of silence (29–30 ms measured,
`millis()` counts whole ms). A window also swallows real events shorter than it (a 24.2 ms gap merged).
Shared ISR data: `volatile` + a `portMUX` spinlock, one snapshot under the lock, `now` read last under it;
`now - last >= interval` survives the 49.7-day `millis()` wrap, `now >= last + interval` doesn't.
`attachInterrupt` goes last in `setup()`: the unlocked initial writes must not race the ISR.
`Serial` from an ISR: its UART lock is a mutex with an infinite wait, so an ISR that interrupts a print
deadlocks. `ARDUINO_ISR_ATTR` is empty in core 2.0.17. `Serial` has no software TX buffer here: a call
returns once the text is in the 128-byte UART FIFO.
`monitor_filters = time` stamps every monitor line, which maps Serial output to edges in a capture
(lines arrive 29–39 ms after the edge).
Steps: `1.5-signals-button/00` lecture (debounce on four channels), `01` homework (counter vs analyzer).

**1.6 — Intro to the ADC, homework "an LDR voltage by formula vs calibration".**
Divider 3V3 → LDR → GPIO 4 → 10 kΩ → GND: more light = lower LDR resistance = higher voltage. Dark (~580 kΩ)
gives ~56 mV, near zero, where a relative error blows up; very bright (~500 Ω) gives ~3.14 V, above the ~3.1 V
11 dB ceiling, so RAW saturates at 4095.
On this board the homework formula RAW / 4095 × 3100 mV reads ~10 % below `analogReadMilliVolts()` from 0.9 to
2.2 V: Ucalc / Umeas ≈ 0.90 in both ranges, a scale factor, not an offset (a "Uref" of ~3.46 V would fit).
Which one is true needs a multimeter. RAW and mV are two separate conversions, so ADC noise (±5 codes) shows
up in the error (±0.2 % in steady light, much more while the light changes).
C traps met: `raw / ADC_MAX` with two integers is 0 (cast one operand to `float` before the division; the
type of the target variable doesn't help, and the compiler doesn't warn); a negative difference in `uint32_t`
wraps to ~4.29 billion; divide only after checking for zero (integer division by zero resets the board).
`uint32_t` is `unsigned int` in core 2.0.17, `unsigned long` in 3.x: `%lu` + a cast prints it on both.
Lecture code: modules split into `.h`/`.cpp` with namespaces; an anonymous namespace makes data file-private;
`constexpr` + `static_assert` reject bad thresholds at build time; a ring-buffer moving average (running sum,
no heap); hysteresis (on below 1600, off above 2200) is a software Schmitt trigger; a non-blocking period with
`millis()`. A window of 8 samples at 1 Hz delayed the LED by 6.4 s (it switched on after the LDR was
uncovered), and sampling once a second missed a half-second change of light.
Logic analyzer: digital only, so it can't read the ADC voltage; it decoded the UART text instead (a 38-character
line takes 3.3 ms at 115200). On the LDR node a slow threshold crossing gave thousands of edges, the ADC
sampling dipped the node for ~150 ns once per loop (likely the sample capacitor), and spikes came every 20 ms
while covered (source unknown). A 100 nF capacitor from the ADC pin to GND is the usual cure (not tried).
Wokwi: no bare LDR, only a module with its own 10 kΩ on the other side (more light = lower voltage); resistor
dividers aren't simulated (two 10 kΩ read 4095); the ADC is ideal (3.3 V = 4095) and `analogReadMilliVolts()`
reads unrealistically high, so use Wokwi to run the code, not for the numbers.
Steps: `1.6-adc-light/00` lecture (smart light, analyzer capture), `01` homework (formula vs calibration).

## Known issues / open

1. 1.3: add "▶ Simulate in Wokwi" links to each step README and the lesson's steps table once
   the steps are published on wokwi.com.
2. 1.3 step 04: the README says the flash length moves "between 100 ms and 15 ms", but the code
   reaches 10 ms: the bound is checked after `flashMs += stepMs`, and 100 − 15 is not a multiple of 10
   (100, 90, …, 20, 10 → flip). Open: fix the code or the README.
3. 1.4: check on the board which GPIOs drive the onboard LEDs (course: 48 = blue SYS-LED, 47 = WS2812).
   1.3 drives external LEDs on both pins, so the onboard LEDs may blink along.
4. 1.6 step 00: `diagram.png` is missing (the README links to it); take it from the Wokwi editor.
5. 1.6: a multimeter on GPIO 4 should decide between the formula and `analogReadMilliVolts()` (~10 % apart),
   and a 100 nF capacitor on the ADC pin should be tried against the noise and the ~150 ns dips.
