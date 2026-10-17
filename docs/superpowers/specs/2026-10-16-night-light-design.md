# Night light with motion sensing and a Wi-Fi page — design

Lesson 1.7 mini project, Module 1, EMB27 course. Status: approved by the author 2026-10-16;
2026-10-17: phone control reduced to the mode (no boost), motion timeout 15 s, the load is an LED strip. Plan: `docs/superpowers/plans/2026-10-17-night-light.md`.

## Intent

Lesson 1.7 asks for a twilight switch: an LDR divider on an ADC pin, a relay driven through a BC547 transistor
stage, and two thresholds with hysteresis. This project does that assignment first, unchanged, and then grows it
into a motion-activated night light that can be watched and controlled from a phone on the home Wi-Fi.

Why it is built this way:
- the base step is the homework that gets handed in, so it must match the assignment exactly;
- every extra step adds one new idea on top of a version that already works on the real board;
- it reuses what the course has covered so far (GPIO, interrupts, debounce and quiet windows, the ADC,
  non-blocking timing with `millis()`, the lesson 1.6 lecture code) and adds two new things: a relay behind a
  transistor, and the board as a small web server.

## The assignment (lesson 1.7, translated in full)

Title: "Mini project: connecting a sensor and an actuator, debugger" — "Twilight switch on the ESP32-S3".
Goal: learn to read an analog signal from a voltage divider (ADC) and to control an external load through a
transistor switch (GPIO).

1. **Circuit.** Build it on the breadboard from the drawing. Input (photoresistor): the divider R1 (LDR) + R2
   (10 kΩ) on the +3.3 V rail, the midpoint to "GPIO In" (ADC). Switch ("matching 3.3 V → 5 V"): "GPIO Out"
   through R3 (10 kΩ) to the base of a BC547B (VT1); the collector pulled up to +5 V through R4 (10 kΩ) and
   connected to the relay module's IN. Output (load): any safe low-voltage load (an LED, a 5 V/12 V LED strip)
   on the relay's OUT NO.
2. **Program**, three steps in the main loop: read the ADC from GPIO In (0…4095); compare with thresholds
   (hysteresis): ADC < `THRESHOLD_DARK` (dark) → HIGH on GPIO Out (relay on); ADC > `THRESHOLD_LIGHT` (light) →
   LOW (relay off); between the thresholds → change nothing ("this protects the relay from chatter").

That is the whole text: it names no pins, no sampling period, no Serial format and nothing else to hand in.
The "debugger" in the title is not described; if the lecture brings it (the ESP32-S3 has a built-in USB-JTAG
on the native USB port), it can become its own step.

## Goals

1. **Base (homework 1.7, as given):** read the ADC (0…4095) from the LDR divider; ADC < `THRESHOLD_DARK` →
   GPIO Out HIGH (relay on); ADC > `THRESHOLD_LIGHT` → LOW (relay off); in between → no change.
2. **Motion:** the lamp turns on only when it is dark **and** a PIR sensor sees motion; it turns off when no motion
   has been seen for a timeout (default 15 s); every new motion restarts the timeout.
3. **Wi-Fi page, read-only ("variant A"):** a page served by the board on the home network shows the light level,
   how long ago motion was seen, the lamp state with the time left, a live chart of the light level and the last
   10 events.
4. **Wi-Fi page, control ("variant B"):** the page also switches the mode (auto / always on / always off).

## Non-goals (later, not now)

- Access from outside the home network (Telegram bot, MQTT via a cloud broker).
- Settings changed from the page and saved in flash: threshold and timeout sliders, "set dark/light = now"
  calibration ("variant C"). It needs flash storage and validation of values from the network.
- OLED screen, rotary encoder (course modules 3–4).
- OTA firmware updates, HTTPS, a login on the page, FreeRTOS tasks.

## Anti-goals (a failure even if it works)

- Wi-Fi credentials in the public repository, in any commit.
- `delay()`-based blocking in the main loop; the web server and the lamp logic must both stay responsive.
- Relay chatter: the relay clicking on and off in quick succession at dusk or on a noisy reading.
- The lamp logic depending on Wi-Fi: with the network down, or before it connects, the night light still works.
- Code that only works in Wokwi.
- Mains voltage anywhere on the breadboard. The load is low-voltage only.
- Breaking the repository conventions: multiple PlatformIO environments, `src_dir`, `build_src_filter`,
  non-English text in the repo.

## Constraints

- Board: ESP32-S3 DevKitC-1 clone (N16R8), Arduino core **2.0.17** through PlatformIO
  (`esp32-s3-devkitc-1`, `framework = arduino`). Check APIs against 2.0.17, not 3.x.
- Built-in libraries only unless there is a strong reason: `WiFi`, `WebServer`, `ESPmDNS` ship with the core.
- Every project is an ordinary wizard project with `build_src_flags = -Wall -Wextra`; the build must have no
  warnings in our code.
- No host-side unit-test environment: the repository allows one PlatformIO environment per project.
  Verification is: a clean build, a Wokwi run where the simulator can show the behaviour, a run on the board with
  the Serial Monitor, and logic analyzer captures (an 8-channel Saleae Logic clone, Logic 2).
- Repository workflow (see `CLAUDE.md` at the repo root and `module-1/CLAUDE.md`): work happens in a lesson
  draft project; finished pieces are copied to `archive/`; `/save-hw` turns them into numbered step projects
  with Wokwi circuits and READMEs; `/finish-lesson` closes the lesson. Commit only after the author agrees.
  Follow the collaboration rules from the CLAUDE.md files loaded in the session.

## Hardware

From the lesson 1.7 schematic, plus the PIR:

```
3V3 ── LDR ──┬── GPIO 4 (ADC1)            GPIO Out ── 10 kΩ ── B  BC547B
             │                                                  E ── GND
           10 kΩ                           5V ── 10 kΩ ──┬── C
             │                                           └── relay module IN
            GND                            relay module VCC = 5V (board 5V pin), GND common

PIR HC-SR501: VCC = 5V, GND, OUT (3.3 V logic) → a GPIO input
Load: a plain LED strip (5 V or 12 V, see its marking) on the relay module's COM / NO screw terminals,
      powered by its own supply (never the board's 5V pin or the MB-102: a strip draws far more current)
```

- LDR on GPIO 4: ADC1 keeps working with Wi-Fi on (ADC2 is shared with Wi-Fi).
- GPIO Out and the PIR input are the builder's choice, avoiding 0/3/45/46 (strapping), 19/20 (native USB),
  26–37 (flash/PSRAM) and 43/44 (UART0).
- The NPN stage inverts: GPIO HIGH → transistor on → IN ≈ 0 V. With a **low-level-trigger** relay module (as
  drawn: a PNP inside) the two inversions cancel and GPIO HIGH = relay on. The stage exists because a 3.3 V HIGH
  straight on IN of a 5 V module doesn't release a PNP input.
- Optional: 100 kΩ from the BC547 base to GND, so the relay doesn't click while GPIO Out floats during boot.
- Available parts: the course kit (relay module, BC547B, LDR, resistors, LEDs, breadboard supply, multimeter)
  a second kit with a 5 V 2-channel relay module and an HC-SR501 PIR, and an LED strip with its own supply.

### Check before wiring (unknown facts; record the results in the step README)

1. **The relay module's trigger level.** Look for an H/L jumper, the input transistor marking (S8550 = PNP,
   low-level; S8050 = NPN, high-level) or an optocoupler. On the 2-channel module keep the JD-VCC jumper on.
   With a high-level-trigger module the NPN stage makes GPIO HIGH = relay **off**: then set the jumper to L if
   there is one, or invert the output in code through one `config.h` constant (builder's call which).
2. **BC547 pinout:** TO-92, flat side facing you, legs down: C B E. Confirm with the multimeter's diode test.
3. **PIR:** after power-up it needs 30–60 s to settle and may trigger falsely meanwhile; its own hold-time
   potentiometer goes to the minimum and its retrigger jumper to "repeatable" (H), so the firmware's timeout is
   the one that counts.

## Behaviour

**Light.** Sample the ADC every 100 ms. Dark/light uses the two thresholds with hysteresis: dark below
`THRESHOLD_DARK`, light above `THRESHOLD_LIGHT`, no change in between; `static_assert(THRESHOLD_DARK <
THRESHOLD_LIGHT)`. Start from the lesson 1.6 values (1600 / 2200) and set the real ones from Serial readings on
this breadboard. Every step compares the raw reading, as the assignment says: the hysteresis already covers the
ADC noise. A moving average (the 1.6 lecture code) is added only if a passing shadow switches the lamp on in the
board test (decided 2026-10-17).

**Lamp, auto mode** (decided with the author):
- switches on when it is dark and motion is seen;
- switches off when the timeout runs out without motion; every motion while on restarts the timeout;
- **light only prevents switching on.** While the lamp is on, the light level is not a reason to switch off: the
  lamp's own light reaching the LDR must not turn it off (that would loop on → light → off → dark → on, which is
  relay chatter). In practice the light reading matters only while the lamp is off, as in commercial motion
  night lights. Motion seen while it is light and the lamp is off does nothing.

**Modes (variant B):** `auto` (above), `on` (always on), `off` (always off). In `on` and `off` the sensors don't
matter; switching back to `auto` applies the auto rule at once. The mode starts as `auto` after a reset. A
"turn on for N minutes" boost was considered and dropped (2026-10-17).

**Events:** the last 10 changes, e.g. "lamp on (dark + motion)", "lamp off (no motion 15 s)", "mode → on",
"Wi-Fi connected". Timestamps as uptime by default; real clock time via SNTP is the builder's option
in the Wi-Fi steps.

**Serial:** one status line per second (light, dark/light, motion age, lamp, mode) plus one line per event, so a
capture and the monitor can be matched.

## Architecture

Modules split into `.h` / `.cpp` with namespaces, as in the lesson 1.6 lecture code:

| Module | Job | Interface (indicative) |
|---|---|---|
| `include/config.h` | pins, thresholds, timeouts, `static_assert`s | constants |
| `light_sensor` | ADC sampling, dark/light hysteresis | `begin()`, `sample(now)`, `isDark()`, `raw()` |
| `motion` | PIR input, time of the last motion | `begin()`, `read(now)`, `msSinceMotion(now)` |
| `lamp` | modes, timeout; **the only code that writes the relay pin** | `begin()`, `update(now)`, `setMode()`, state getters |
| `event_log` | ring buffer of the last 10 events | `add()`, `list()` |
| `web` | Wi-Fi, reconnect, mDNS, routes; reads state, sends commands to `lamp`, never touches pins | `begin()`, `handle()` |
| `page.h` | the page (HTML + JS) as a raw string literal | constant |

`loop()` is a non-blocking superloop: sample the light (every 100 ms) → read motion → `lamp.update(now)` →
`web.handle()`.

**Web, decided:**
- Station mode on the home network. Credentials in `include/secrets.h`, ignored by git (add it to the root
  `.gitignore` before the file exists); `include/secrets.example.h` with placeholders is committed.
- The board prints its IP on Serial and answers as `http://nightlight.local` (mDNS).
- Reconnect without blocking when Wi-Fi drops (check every few seconds); lamp logic keeps running meanwhile.
- Routes:
  - `GET /` → the page;
  - `GET /api/state` → JSON built with `snprintf`, e.g.
    `{"raw":1180,"dark":true,"motionAgoMs":12000,"lamp":true,"leftS":18,"mode":"auto","events":[…]}`;
  - `POST /api/mode?m=auto|on|off` (variant B).
  Any other value of `m`, or no `m` → HTTP 400 and no change.
- The page is self-contained (no CDN): it polls `/api/state` once a second and keeps the chart history itself
  (the board stores no history beyond the 10 events). UI text in English.

**Left to the builder:** the remaining pin choices, file names beyond the table, the exact JSON field names, the
page layout and chart technique (canvas or SVG), SNTP or uptime, the mDNS fallback if
`.local` doesn't resolve on the phone.

## Steps

Numbered in the order of the work, as the repository requires. Suggested lesson folder:
`module-1/1.7-twilight-switch/`.

Save each step into its numbered folder as soon as it works on the board, and commit it then, as lessons 1.5
and 1.6 did: the base step can be handed in before the extras exist. The draft stays in the lesson folder (and in
the workspace) until `/finish-lesson`; nothing is numbered ahead of time, so lecture code that arrives later simply
takes the next free number.

| Step | What | Done when |
|---|---|---|
| `base-twilight-switch` | the assignment as given: LDR, thresholds with hysteresis, relay through BC547 | covering the LDR switches the relay on, light switches it off, no chatter at the thresholds |
| `extra-motion` | PIR + timeout in auto mode | lamp on only in the dark with motion, off 15 s after the last motion, motion restarts the countdown; the strip shining on the LDR doesn't switch it off |
| `extra-web-status` | Wi-Fi + read-only page (variant A) | the phone shows live values, chart and events; the lamp still works with Wi-Fi off |
| `extra-web-control` | modes (variant B) | the mode works from the phone; a bad `m` gets 400 |

## Verification per step

- `pio run` with no warnings in our code.
- Wokwi where it helps: it has a PIR and a relay module part, but its photoresistor module is wired the other way
  round (more light = lower voltage), it doesn't solve resistor dividers, and it has no transistor model as far as
  known, so its relay may be driven straight from the GPIO with a README note. Wokwi also simulates Wi-Fi.
- On the board: Serial status lines; a multimeter on GPIO 4 for the light levels used as thresholds.
- Logic analyzer captures (channels: relay IN, PIR OUT, GPIO Out, UART TX): no relay chatter, the timeout
  measured from the last PIR edge to the relay edge (15 s), the reaction time from "dark + motion" to the relay.
- Wi-Fi steps: page from the phone; Wi-Fi drop (router off/on or a wrong password) → the lamp keeps working and the
  board reconnects; invalid commands are rejected.
- Each step README: what it does, wiring table, the text schematic, Wokwi notes, measured results, what breaks on
  real hardware.
