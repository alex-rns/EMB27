# Night Light Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the lesson 1.7 twilight switch exactly as assigned, then grow it in three saved steps into a
motion-activated night light with a read-only page on the home Wi-Fi and then a mode switch (auto / on / off) on
that page.

**Architecture:** One lesson draft (an ordinary PlatformIO project at the root of `module-1/1.7-twilight-switch/`,
as in lesson 1.6) grows in four stages. Each stage that works on the board is copied into a numbered step project
and committed. Modules are split into `.h`/`.cpp` with namespaces (lesson 1.6 lecture style), `loop()` is a
non-blocking `millis()` superloop, `Lamp` is the only code that writes the relay pin, and `Web` only reads state
and sends commands to `Lamp`.

**Tech Stack:** ESP32-S3 DevKitC-1 clone (N16R8), PlatformIO board `esp32-s3-devkitc-1`, Arduino core 2.0.17
(`framework-arduinoespressif32` 4.20017), built-in `WiFi` / `WebServer` / `ESPmDNS`; Wokwi (VS Code and MCP),
Logic 2 (MCP), a multimeter.

**Spec:** `docs/superpowers/specs/2026-10-16-night-light-design.md`

## Global Constraints

- Arduino core **2.0.17**: check every API against the installed core
  (`~/.platformio/packages/framework-arduinoespressif32/`), never against 3.x docs.
- Built-in libraries only: `WiFi`, `WebServer`, `ESPmDNS`. No `lib_deps`.
- Every project has one `[env:esp32-s3-devkitc-1]` with `platform = espressif32`, `board = esp32-s3-devkitc-1`,
  `framework = arduino`, `build_src_flags = -Wall -Wextra`, `monitor_speed = 115200`, `monitor_filters = time`.
  Never multiple environments, `src_dir` or `build_src_filter`. The build has no `warning:` lines from our code.
- No `delay()` in `loop()` or anything it calls. Every period is `now - last >= PERIOD` (survives the `millis()`
  wrap); never `now >= last + PERIOD`.
- The lamp logic never depends on Wi-Fi: `setup()` returns without waiting for a connection.
- Wi-Fi credentials live only in `include/secrets.h`, which git ignores. The root `.gitignore` gets `secrets.h`
  **before** any such file exists. The executor never reads `secrets.h`; the author types it.
- Everything in the repo is in English; firmware strings and page text are ASCII and in English.
- Low voltage only: no mains anywhere on the breadboard.
- Pins: LDR divider midpoint → **GPIO 4** (ADC1); GPIO Out (to the BC547 base resistor) → **GPIO 7**;
  PIR OUT → **GPIO 5**.
- `THRESHOLD_DARK = 1600`, `THRESHOLD_LIGHT = 2200` to start, `static_assert(THRESHOLD_DARK < THRESHOLD_LIGHT)`;
  the real values come from Serial readings on this breadboard (Task 2).
- ADC sample every **100 ms**; one Serial status line every **1000 ms** plus one line per event; motion timeout
  **15 s**; event log keeps the last **10** events; uptime timestamps. Phone control is the mode only: no boost.
- The logic analyzer takes 0–3.3 V and has no overvoltage protection (course hardware library,
  `components/logic-analyzer/`). **Never** connect a channel to relay IN or the BC547 collector (5 V).
  Channels: D0 = GPIO 7 (GPIO Out), D1 = GPIO 5 (PIR OUT), D2 = GPIO 43 (UART0 TX); analyzer GND to board GND.
- Who does what: the author does every hands-on action (wiring, Upload, Serial Monitor, multimeter, phone), one
  action at a time; the executor writes the code, builds, runs Wokwi and Logic 2 through MCP, runs `curl`, and
  reads what the author pastes.
- Every task ends with a commit and push of what it produced: the executor suggests the message
  (`<folder>: <what was done>`), the author says yes. From Task 5 on, NOSECRET runs before every commit.

## How a task is verified (no unit tests)

The spec allows no host test environment: one PlatformIO environment per project. In each task the "failing
test" is a check written down before the code: exact Serial lines in a Wokwi run, a Logic 2 measurement, a `curl`
status. It fails first because the draft doesn't do it yet, and passes once the code is in. Named checks:

- **BUILD** `<proj>`: `rm -rf <proj>/.pio/build/*/src; ~/.platformio/penv/bin/pio run -d <proj> 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'`
  → only `SUCCESS`.
- **NOBLOCK** `<proj>`: `grep -rnE 'delay\(|now(Ms)? *>=? *[A-Za-z_]+ *\+' <proj>/src <proj>/include` → nothing.
- **WIRES** `<proj>`: the local Wokwi wire checker (not part of the repo) on `<proj>/diagram.json` → no wire
  reported; `python3 -m json.tool <proj>/diagram.json > /dev/null`.
- **NOCYR**: the Cyrillic check from `.claude/skills/save-hw/SKILL.md` step 5 → nothing.
- **NOSECRET**: `git ls-files | grep secrets` → only `…/secrets.example.h` paths;
  `git check-ignore -v <proj>/include/secrets.h` → matched by the root `.gitignore`.
- **WOKWI**: `wokwi_start_simulation(projectPath=<proj>)` after BUILD; plan every `set_control` / `read_serial`
  call up front, keep text between calls short (the simulation runs in real time), stop it right after. For
  timing use `wokwi_export_vcd`, not `read_pin`.

## Save-step procedure (SAVE)

Used by Tasks 2, 4, 6 and 8. It follows `/save-hw` steps 3–5, with one change from the spec: the draft stays in
the lesson folder and in the workspace until `/finish-lesson` (no Trash in between).

1. In the draft, copy `include/` and `src/` to `archive/<type>-<slug>/` (never `secrets.h`).
2. Create `NN-<type>-<slug>/`, NN = the next free number (expected 00, 01, 02, 03; lecture code that arrives
   first takes the next number): `platformio.ini` (the constraint lines, no wizard comments), `wokwi.toml`
   (`/save-hw` template), `include/` + `src/` from the archive copy, `diagram.json` from the draft, a step
   `README.md`: `# NN — <Type>: <title>`, what the step does, a `| Part | GPIO |` table, the text schematic,
   `![Schematic](schematic.svg)` (schemdraw, as in `/save-hw`), `![Circuit in Wokwi](diagram.png)`, Wokwi notes, **Measured on the board** (this task's results), **Notes**
   (what breaks on real hardware). A Logic 2 capture goes next to it as `capture.sal`.
3. Ask the author for `diagram.png` (a screenshot from the Wokwi editor); check it shows the `diagram.json` wiring.
4. Register: the lesson `README.md` steps table; the `module-1/README.md` row →
   `[Twilight switch → night light](1.7-twilight-switch/) — base task + K extras`; `module-1/CLAUDE.md` lessons
   row → `1.7-twilight-switch` + `✅ saved NN · ⏳ /finish-lesson`, and "Pins used so far" → `1.7: 4, 5, 7`.
5. BUILD the new step, NOBLOCK, WIRES, NOCYR, NOSECRET, `git status --short` (no `.pio/`, `.vscode/`,
   project `.gitignore`, `secrets.h`).
6. Suggest `module-1/1.7-twilight-switch: save step NN-<type>-<slug>`; commit and push after the author's yes.

## Review Focus

1. **PIR wire loose or unplugged** → the input reads "no motion", never random motion: `INPUT_PULLDOWN` on
   GPIO 5. Test: Task 4 Step 7.
2. **No Wi-Fi at boot (wrong password, router off)** → the lamp reacts to dark + motion within the first seconds
   and the status line never pauses. Test: Task 6 Step 7.
3. **A mode value that only looks valid** (`m=ON`, `m=`, `m=auto ` with a trailing space, no `m`) → 400 and no
   change: only an exact, case-sensitive match counts. Test: Task 8 Step 2.
4. **The page loses the board** (board reset, phone leaves the Wi-Fi) → the page says "offline" within 3 s, keeps
   its chart and recovers by itself. Test: Task 6 Step 5.
5. **Switching back to auto** → the auto rule applies at once: from `on` with no motion for 15 s (or at once when it is light) the lamp goes off,
   from `off` in the dark with motion it goes on (nobody expects to wait for the next change). Test: Task 8 Step 3.

---

### Task 1: Lesson draft with the base firmware (twilight switch)

**Files:**
- Create: `module-1/1.7-twilight-switch/` with
  `~/.platformio/penv/bin/pio project init -d module-1/1.7-twilight-switch -b esp32-s3-devkitc-1 -O framework=arduino`
  (the wizard layout), then add the constraint lines to its `platformio.ini` with one short comment each, as the
  1.6 draft had.
- Create: `include/config.h`, `include/light_sensor.h`, `src/light_sensor.cpp`, `include/lamp.h`, `src/lamp.cpp`,
  `src/main.cpp`, `diagram.json`, `wokwi.toml` (all inside the draft).
- Modify: `EMB27.code-workspace`: the `1.6 / 01-base-voltage-error` folder → `"name": "1.7 (draft)"`,
  `"path": "module-1/1.7-twilight-switch"`.

**Interfaces:**
- Produces `namespace Config` (`config.h`): `uint8_t LDR_GPIO = 4`, `RELAY_GPIO = 7`; `uint32_t BAUD = 115200`,
  `SAMPLE_PERIOD_MS = 100`, `STATUS_PERIOD_MS = 1000`; `uint16_t THRESHOLD_DARK = 1600`, `THRESHOLD_LIGHT = 2200`;
  `bool RELAY_ON_IS_HIGH = true` (the GPIO level that switches the relay on through the BC547 stage); all
  `constexpr`; `static_assert(THRESHOLD_DARK < THRESHOLD_LIGHT)`, `static_assert(THRESHOLD_LIGHT <= 4095)`.
  Names follow the assignment (`THRESHOLD_DARK`), so all constants are UPPER_SNAKE.
- Produces `namespace LightSensor`: `void begin()` (12 bits, `ADC_11db` on GPIO 4); `bool sample(uint32_t nowMs)`
  (reads once per `SAMPLE_PERIOD_MS`, true when it read); `uint16_t raw()`; `bool isDark()`: the hysteresis
  state, raw < `THRESHOLD_DARK` → dark, raw > `THRESHOLD_LIGHT` → light, otherwise unchanged; it starts as light.
- Produces `namespace Lamp`: `void begin()`: writes the off level **before** `pinMode(OUTPUT)`, so an inverted
  relay doesn't click at boot; `void update(uint32_t nowMs)`: base rule, on while `LightSensor::isDark()`, writes
  the pin only on a change and prints one event line; `bool isOn()`.
- Serial: status `raw: 2650 | light | relay: off`; event `[   12.3 s] relay on (dark)` / `[   12.3 s] relay off (light)`.

- [ ] **Step 1: Write the Wokwi checks down** (they fail: the draft doesn't exist). The circuit: `wokwi-potentiometer`
  `pot1` stands in for the LDR divider (VCC → 3V3, SIG → GPIO 4, GND; higher = more light, the right direction,
  unlike Wokwi's photoresistor module); `wokwi-relay-module` `relay1` driven straight from GPIO 7 (Wokwi has no
  transistor model); an LED + 220 Ω on the relay's COM/NO stands in for the LED strip.
  - B1: boot with `pot1` at mid-range between the thresholds → `| light | relay: off`, no event.
  - B2: `pot1` low (< 1600) → `relay on (dark)` within 1 s, status `| dark | relay: on`.
  - B3: back to mid-range → no event for 3 s, still `relay: on`.
  - B4: `pot1` high (> 2200) → `relay off (light)`.
  - B5: back to mid-range → no event for 3 s, still `relay: off`.
- [ ] **Step 2: Create the draft** with `pio project init` (command above), add the constraint lines to
  `platformio.ini`, update `EMB27.code-workspace`.
- [ ] **Step 3: Implement `config.h`, `light_sensor`, `lamp`** with the interfaces above.
- [ ] **Step 4: Implement `main.cpp`**: `setup()`: `Serial.begin`, `Lamp::begin()`, `LightSensor::begin()`;
  `loop()`: `now = millis()` → `LightSensor::sample(now)` → `Lamp::update(now)` → the status line every
  `STATUS_PERIOD_MS`.
- [ ] **Step 5: Write `diagram.json` and `wokwi.toml`**; check part pin names on `docs.wokwi.com/parts/<type>`.
  Run WIRES. Expected: no wire reported, valid JSON.
- [ ] **Step 6: Run BUILD and NOBLOCK on the draft.** Expected: `SUCCESS` only; NOBLOCK prints nothing.
- [ ] **Step 7: Run WOKWI with B1–B5.** Find the `set_control` value scale of `pot1` from the first reading.
  Expected: every line as written in Step 1.
- [ ] **Step 8: Stop.** Walk the author through the code module by module, plus what can break on real
  hardware.
- [ ] **Step 9: Commit and push** the draft and `EMB27.code-workspace`:
  `module-1/1.7-twilight-switch: add the draft with the base firmware`.

### Task 2: Base on the board, save `00-base-twilight-switch`

**Files:**
- Modify: `include/config.h` in the draft (calibrated thresholds; `RELAY_ON_IS_HIGH` if the module is high-level).
- Create: `module-1/1.7-twilight-switch/README.md` (lesson): the assignment in English (the spec's
  "The assignment"), the circuit, the steps table, "Working with the steps" (shape of `1.6-adc-light/README.md`).
- Create: step `00-base-twilight-switch/` through SAVE.

**Interfaces:** Consumes Task 1. Produces calibrated `THRESHOLD_DARK` / `THRESHOLD_LIGHT` used by every later step.

- [ ] **Step 1: Checks before wiring** (the author, multimeter; results go to the step README):
  relay module trigger level (H/L jumper, S8550 = PNP low-level / S8050 = NPN high-level, optocoupler, JD-VCC
  jumper on); BC547 pinout C-B-E (diode test, red probe on B: B→C and B→E ≈ 0.6–0.7 V); the board's 5V pin to
  GND with USB plugged in ≈ 4.6–5.1 V (this clone's IN-OUT jumper decides whether the pin carries USB power).
  A high-level module → set its jumper to L, or `RELAY_ON_IS_HIGH = false` (the author's call). The LED strip:
  its voltage (5 or 12 V, printed on it) and a supply of that voltage that gives enough current (the strip's
  marking, or its length × watts per metre); never the board's 5V pin or the MB-102.
- [ ] **Step 2: Wiring**, one block at a time, each checked before the next: the divider on GPIO 4; GPIO 7 →
  10 kΩ → B, E → GND, 5V → 10 kΩ → C, C → relay IN; relay VCC → 5V (the breadboard supply: on this clone the board's 5V pin has no USB power), GND → GND; load: the strip's supply + → COM,
  NO → strip +, strip − → supply −, straight into the module's screw terminals, not through the breadboard
  (its contacts are not made for amps). Before USB: the multimeter shows no short 5V–GND and 3V3–GND.
- [ ] **Step 3: Upload, open the Serial Monitor.** Expected: a status line per second; covering the LDR → a click,
  the relay LED and the strip on, `relay on (dark)`; light again → `relay off (light)`.
- [ ] **Step 4: Calibrate.** The author pastes `raw` in room light, shaded by a hand at ~10 cm, and fully covered.
  Pick `THRESHOLD_DARK` / `THRESHOLD_LIGHT` with a gap of ≥ 300 codes (the ADC noise is about ±5), rebuild,
  re-upload. The multimeter on GPIO 4 at both thresholds gives the volts for the README.
- [ ] **Step 5: Chatter capture.** Logic 2: timed 30 s, ≥ 1 MS/s, D0 + D2 (UART decoded). The author slowly covers
  and uncovers the LDR three times. Expected: exactly 6 edges on D0, none less than 1 s apart, each matching an
  event line in the UART decode. Save as `capture.sal`.
- [ ] **Step 6: Boot click.** The author presses RST three times and watches the relay LED. A click at boot →
  100 kΩ from the BC547 base to GND, then retest. Record the result.
- [ ] **Step 7: SAVE as `00-base-twilight-switch`** (type base), together with the lesson README. README notes:
  the double inversion (NPN stage + low-level module), why 3.3 V straight on IN doesn't release the relay, the
  relay's contact life, and the Wokwi stand-ins (the pot, the relay without a transistor).

### Task 3: Motion firmware (PIR + 15 s timeout)

**Files:**
- Create: `include/motion.h`, `src/motion.cpp`.
- Modify: `include/config.h`, `include/light_sensor.h`, `src/light_sensor.cpp`, `include/lamp.h`, `src/lamp.cpp`,
  `src/main.cpp`, `diagram.json` (all in the draft).

**Interfaces:**
- Consumes: `LightSensor::*`, `Lamp::begin/isOn` (Task 1).
- Produces `Config`: `uint8_t PIR_GPIO = 5`, `uint32_t MOTION_TIMEOUT_MS = 15000`.
- Produces `namespace Motion`: `constexpr uint32_t NEVER = UINT32_MAX`; `void begin()` (`INPUT_PULLDOWN`);
  `bool read(uint32_t nowMs)`: reads the pin, true while PIR OUT is HIGH, and stores `nowMs` as the last motion
  time on every HIGH reading; `bool active()`: the last reading; `uint32_t msSinceMotion(uint32_t nowMs)`:
  `NEVER` until the first motion, 0 while active. So the timeout runs from the PIR **falling** edge.
- `LightSensor` stays as in Task 1: the hysteresis works on the raw reading, with no moving average. The hysteresis
  already covers the ADC noise; Task 4 Step 6 decides whether a passing shadow needs one.
- Produces `uint32_t Lamp::secondsLeft(uint32_t nowMs)`: 0 when off, otherwise the seconds until the auto-off,
  rounded up. `Lamp::update(nowMs)` now runs the auto rule, the one decision the spec fixes:
  ```
  if (!on) { if (LightSensor::isDark() && Motion::active()) switchOn("dark + motion"); }
  else if (Motion::msSinceMotion(now) >= Config::MOTION_TIMEOUT_MS) switchOff("no motion 15 s");
  ```
  (Amended 2026-10-18: the draft now also switches off when `!LightSensor::isDark()`, event `lamp off (light)`; see the spec.) The "15" in the event comes from `MOTION_TIMEOUT_MS / 1000`.
- Serial: status `raw: 812 | dark | motion: 4.2 s ago | lamp: on, 11 s left` (`motion: never`,
  `motion: now`); events `lamp on (dark + motion)`, `lamp off (no motion 15 s)`.

- [ ] **Step 1: Write the Wokwi checks down.** The circuit adds `wokwi-pir-motion-sensor` `pir1` (VCC → 5V,
  OUT → GPIO 5, GND), its `delayTime` at the minimum. If MCP `set_control` can't trigger it, the author clicks
  the PIR in VS Code when asked.
  - M1: `pot1` high (light) + motion → no lamp event.
  - M2: `pot1` low (dark), no motion for 5 s → the lamp stays off.
  - M3: dark + motion → `lamp on (dark + motion)` within 1 s.
  - M4: `pot1` high while on → still `lamp: on` (the lamp's own light on the LDR).
  - M5: VCD: PIR OUT falling edge → GPIO 7 falling edge = 15.0 s ± 0.1 s, with `lamp off (no motion 15 s)`.
  - M6: motion again ~8 s into a countdown → `left` jumps back to 15 s; off 15 s after the **last** falling edge.
- [ ] **Step 2: Implement `Motion`** and the new `Config` constants.
- [ ] **Step 3: Implement the auto rule and `Lamp::secondsLeft()`**; rename the events to `lamp …`.
- [ ] **Step 4: Update `main.cpp`**: `Motion::begin()` in `setup()`; `Motion::read(now)` before `Lamp::update(now)`;
  the new status line.
- [ ] **Step 5: Add `pir1` to `diagram.json`.** Run WIRES. Expected: no wire reported.
- [ ] **Step 6: Run BUILD and NOBLOCK.** Expected: `SUCCESS` only; nothing from NOBLOCK.
- [ ] **Step 7: Run WOKWI with M1–M6.** Expected: as written in Step 1.
- [ ] **Step 8: Stop.** Walk the author through the diff.
- [ ] **Step 9: Commit and push**: `module-1/1.7-twilight-switch: draft: add the PIR and the 15 s timeout`.

### Task 4: Motion on the board, save `01-extra-motion`

**Files:** Create step `01-extra-motion/` through SAVE. Modify the draft only if a check fails.

**Interfaces:** Consumes Task 3.

- [ ] **Step 1: PIR setup** (the author): hold-time potentiometer to the minimum, retrigger jumper on H
  (repeatable), sensitivity in the middle. Power it (VCC → the breadboard supply's 5 V, GND) **without** OUT connected; the multimeter on
  OUT shows HIGH ≈ 3.3 V (never 5 V) when a hand waves. Only then OUT → GPIO 5.
- [ ] **Step 2: Upload, wait 60 s** (PIR warm-up), Serial Monitor. Expected: `motion: now` while waving.
- [ ] **Step 3: Timeout capture.** Logic 2: timed 45 s, ≥ 1 MS/s, D0, D1, D2. In the dark the author waves once,
  then waits. Expected: D1↑ → D0↑ in < 10 ms (the reaction time); D1's last ↓ → D0↓ = 15.0 s ± 0.05 s.
  Save as `capture.sal`.
- [ ] **Step 4: Restart capture.** Same setup; a second wave about 8 s into the countdown. Expected: D0↓ 15 s
  after the second wave's D1↓, not the first.
- [ ] **Step 5: Own-light test.** The strip shines on the LDR. Dark + motion → lamp on; Serial `raw` rises above
  `THRESHOLD_LIGHT`; the lamp stays on until the timeout; exactly one on/off pair, no re-on without new motion.
- [ ] **Step 6: Passing shadow.** In room light, a hand passes over the LDR five times while the PIR sees it.
  Expected: no `lamp on`. If the lamp does switch on, add the moving average from the 1.6 lecture code (8 samples,
  0.8 s) to `LightSensor`, run the test again, and record the reason in the README.
- [ ] **Step 7: Loose PIR wire** (Review Focus 1). In the dark, OUT unplugged from GPIO 5 for 60 s. Expected:
  no motion is seen, the lamp stays off.
- [ ] **Step 8: SAVE as `01-extra-motion`** (type extra). README notes: the PIR warm-up false triggers, its own
  hold time adding to the 15 s, why light only blocks switching on (the on → light → off → dark loop), the
  passing-shadow result.

### Task 5: Read-only web page (variant A)

**Files:**
- Modify: the root `.gitignore`: `secrets.h` with a one-line comment. **First action of the task.**
- Create: `include/secrets.example.h`: `constexpr char WIFI_SSID[] = "Wokwi-GUEST";`,
  `constexpr char WIFI_PASSWORD[] = "";` and a comment: copy to `secrets.h` and put your network there
  (Wokwi-GUEST is the simulator's open network).
- Create: `include/event_log.h`, `src/event_log.cpp`, `include/web.h`, `src/web.cpp`, `include/page.h`.
- Modify: `include/config.h`, `src/lamp.cpp`, `src/main.cpp`.

**Interfaces:**
- Consumes: `LightSensor::raw/isDark`, `Motion::msSinceMotion/NEVER`, `Lamp::isOn/secondsLeft`.
- Produces `Config`: `const char* HOSTNAME = "nightlight"`, `uint32_t WIFI_CHECK_MS = 5000`,
  `size_t EVENT_COUNT = 10`.
- Produces `struct Event { uint32_t atMs; char text[48]; };` and `namespace EventLog`:
  `void add(uint32_t nowMs, const char* text)` (copies with truncation to 47 characters, drops the oldest beyond
  `EVENT_COUNT`, prints the event line in the Task 1 format); `size_t count()`; `const Event& get(size_t i)`
  (0 = the oldest). `Lamp` now reports its events through `EventLog::add`.
- Produces `namespace Web`: `void begin()`: STA mode, hostname, `WiFi.begin(WIFI_SSID, WIFI_PASSWORD)`, returns
  at once (check the `setHostname` / `mode` order in the 2.0.17 source); `void handle(uint32_t nowMs)`: every
  `WIFI_CHECK_MS`, a change to connected → event `Wi-Fi connected, IP a.b.c.d`, and on the first connect also
  `MDNS.begin(HOSTNAME)`, `MDNS.addService("http", "tcp", 80)`, `server.begin()`; a change to disconnected →
  event `Wi-Fi lost`; while disconnected → `WiFi.reconnect()`. Once the server is running, `server.handleClient()`
  on every call.
- Secrets: `web.cpp` includes `secrets.h` when `__has_include("secrets.h")`, otherwise `secrets.example.h`, and in
  that case prints `Wi-Fi: no secrets.h, using secrets.example.h` once at boot. Builds without `secrets.h` (CI,
  `/finish-lesson`) still succeed.
- Routes: `GET /` → `send_P(200, "text/html", PAGE)` (check `send_P` in 2.0.17 `WebServer.h`);
  `GET /api/state` → JSON; anything unmatched → 404 `text/plain` `not found`.
- JSON, exact keys, events oldest first:
  `{"uptimeMs":123456,"raw":812,"dark":true,"darkBelow":1600,"lightAbove":2200,"motionAgoMs":4200,"lamp":true,"leftS":11,"events":[{"atMs":1000,"text":"lamp on (dark + motion)"}]}`;
  `motionAgoMs` is `null` before the first motion. Built with `snprintf` into a 1024-byte buffer, every return
  value checked; an overflow → 500 `json overflow`.
- `PAGE` (`page.h`): a raw string literal `R"HTML(…)HTML"`, under 8 KB, no external URLs, English text. On top, the
  lamp ON/OFF with a one-line reason derived from the JSON (`it is light, stays off`, `dark, waiting for motion`,
  `motion keeps it on`, `no motion, off in N s`) and a 15 s timer bar. Then the light as DARK/LIGHT with
  `raw N · below 1600` / `above 2200` / `between: keeps state`; motion as `now` / `N s ago` / `none yet`; a canvas
  chart "Light, last 5 min" of the last 300 `raw` values with both threshold lines labelled dark and light; the
  events newest first as "N s ago" (`uptimeMs − atMs`). It fetches `/api/state` every 1000 ms; a failed fetch shows "offline" and keeps the chart,
  the next success clears it.

- [ ] **Step 1: Add `secrets.h` to the root `.gitignore`.** Run `git check-ignore -v module-1/1.7-twilight-switch/include/secrets.h`.
  Expected: `.gitignore:<line>:secrets.h`.
- [ ] **Step 2: Write the checks down.**
  - S1: `grep -nE 'https?://' include/page.h` → nothing.
  - S2: the `<script>` of `PAGE` extracted to the scratchpad, `node --check` → no output.
  - S3: BUILD without `secrets.h` → `SUCCESS`.
  - W1 (Wokwi, example credentials): `Wi-Fi: no secrets.h, using secrets.example.h`, then
    `Wi-Fi connected, IP …` within 15 s; status lines keep coming once a second meanwhile (no gap over 1.1 s).
  - W2: dark + motion at about 1 s after boot → `lamp on (dark + motion)` **before** `Wi-Fi connected`.
- [ ] **Step 3: Implement `EventLog`** and move `Lamp`'s event lines to it.
- [ ] **Step 4: Implement `Web`** with the routes and the JSON.
- [ ] **Step 5: Write `page.h`.**
- [ ] **Step 6: `main.cpp`**: `Web::begin()` last in `setup()`, `Web::handle(now)` last in `loop()`.
- [ ] **Step 7: Run S1, S2, S3 and NOBLOCK.** Expected: as written in Step 2; NOBLOCK prints nothing.
- [ ] **Step 8: Page preview** (optional, the author's call): serve the page and a fixed `api/state` file from
  the scratchpad with `python3 -m http.server`; the author opens it on the Mac, then the server is stopped and
  the page must say "offline".
- [ ] **Step 9: Run WOKWI with W1–W2.**
- [ ] **Step 10: Stop.** Walk the author through `web.cpp`, `event_log.cpp` and `page.h`.
- [ ] **Step 11: NOSECRET, then commit and push** the draft and the root `.gitignore`:
  `module-1/1.7-twilight-switch: draft: add the read-only Wi-Fi page`.

### Task 6: Read-only page on the board, save `02-extra-web-status`

**Files:** Create step `02-extra-web-status/` through SAVE (no `secrets.h` in it; its README says how to make one).

**Interfaces:** Consumes Task 5.

- [ ] **Step 1: Secrets** (the author): `cp include/secrets.example.h include/secrets.h`, then types the home SSID
  and password. Run NOSECRET. Expected: only `secrets.example.h` is tracked; `secrets.h` matches `.gitignore`.
- [ ] **Step 2: Upload, Serial Monitor.** Expected: `Wi-Fi connected, IP …` within 15 s.
- [ ] **Step 3: Phone** at `http://nightlight.local`, and at the IP if `.local` doesn't resolve (record which one
  works). Expected: the values follow a hand over the LDR, the chart moves, events are listed, "s left" counts down.
- [ ] **Step 3a: Own-light loop check** (amendment 2026-10-18). In the dark, lamp on (wave), strip lit. Expected:
  the raw reading stays below 2200 (`THRESHOLD_LIGHT`) for 15 s of Serial lines; no `lamp off (light)`. If raw
  climbs above 2200 the strip reaches the LDR: shield the LDR and repeat. Then switch the room light on while the
  lamp is on: `lamp off (light)`; room light off, wave: `lamp on (dark + motion)`.
- [ ] **Step 4: Responsiveness capture.** Logic 2 as in Task 4 Step 3, with the page open and polling. Expected:
  D1↑ → D0↑ under 50 ms (compare with Task 4).
- [ ] **Step 5: Page loses the board** (Review Focus 4): RST with the page open. Expected: "offline" within 3 s,
  back by itself after the boot, the chart kept.
- [ ] **Step 6: Wi-Fi drop**: router off/on, or a phone hotspot used as the network and switched off. Expected:
  `Wi-Fi lost`; dark + wave still switches the lamp on; `Wi-Fi connected` within ~15 s after the network is back;
  the page works again.
- [ ] **Step 7: Wrong password** (Review Focus 2): a wrong password in `secrets.h`, upload. Expected: dark + wave
  → lamp on within the first 10 s after boot; status lines without a gap. Then restore the password.
- [ ] **Step 8: SAVE as `02-extra-web-status`** (type extra). README notes: ADC1 vs ADC2 with Wi-Fi, `.local` on
  phones, `handleClient()` serving one request at a time, Wi-Fi current peaks on a weak USB cable (brownout), the light-off rule (room light switches the lamp off) and its
  own-light loop risk with the result of Step 3a, brief flashes (car headlights) as an open question.

### Task 7: Mode switch on the page (variant B)

**Files:** Modify `include/lamp.h`, `src/lamp.cpp`, `src/web.cpp`, `include/page.h`, `src/main.cpp` (all in the
draft).

**Interfaces:**
- Consumes: Tasks 3 and 5.
- Produces in `lamp.h`: `enum class Mode : uint8_t { Auto, On, Off };` (`Auto` after every reset, not stored);
  `void setMode(Mode m, uint32_t nowMs)`: applies at once, event `mode -> on` (no event when the mode doesn't
  change); `Mode mode()`; `const char* modeName(Mode m)` → `"auto"` / `"on"` / `"off"`.
- `Lamp::update` order: `On` → on; `Off` → off; `Auto` → the Task 3 rule. Switch events carry the reason:
  `lamp on (mode on)`, `lamp off (mode off)`, and the Task 3 reasons. `secondsLeft()` is 0 outside `Auto`.
- Route: `POST /api/mode?m=auto|on|off`, an exact, case-sensitive match. OK → 200 + the state JSON; anything else
  (no `m`, `m=`, `m=ON`, `m=auto ` with a space) → 400 `text/plain` `bad m: expected auto|on|off` and no change.
  The JSON gains `"mode":"auto"`.
- Page: three mode buttons, the current one highlighted; a press POSTs with `fetch` and shows the 400 text. A line
  under them says what the current mode does (`auto: light and motion decide`, `on: always on, sensors ignored`,
  `off: always off, sensors ignored`); in `on` / `off` the lamp reason reads `mode on: always on` /
  `mode off: always off`.
- Serial status gains `| mode: auto`.

- [ ] **Step 1: Write the checks down.**
  - S1: `node --check` on the page script → no output; `grep -nE 'https?://' include/page.h` → nothing.
  - W1 (Wokwi): the status line ends with `| mode: auto`; M3 from Task 3 still passes.
- [ ] **Step 2: Implement `Mode` and `setMode`** and the new `Lamp::update` order.
- [ ] **Step 3: Implement the POST route** and the `mode` field in the JSON.
- [ ] **Step 4: Add the mode buttons to `page.h`.**
- [ ] **Step 5: Run S1, BUILD and NOBLOCK.** Expected: as written; `SUCCESS` only.
- [ ] **Step 6: Run WOKWI with W1.**
- [ ] **Step 7: Stop.** Walk the author through the diff.
- [ ] **Step 8: NOSECRET, then commit and push**: `module-1/1.7-twilight-switch: draft: add the mode switch`.

### Task 8: Mode switch on the board, save `03-extra-web-control`

**Files:** Create step `03-extra-web-control/` through SAVE.

**Interfaces:** Consumes Task 7.

- [ ] **Step 1: Upload.** The phone page shows the three mode buttons, `auto` highlighted.
- [ ] **Step 2: Request table** with `curl -s -o /dev/null -w '%{http_code}\n' -X POST '<url>'` from the Mac on the
  same network, `/api/state` read before and after every 400:

  | Request | Expected |
  |---|---|
  | `/api/mode?m=on` | 200, the lamp on whatever the light and motion |
  | `/api/mode?m=off` | 200, the lamp off; dark + wave does nothing |
  | `/api/mode?m=auto` | 200, the auto rule again |
  | `/api/mode?m=ON`, `/api/mode?m=`, `/api/mode?m=auto%20`, `/api/mode` | 400, state unchanged |
  | `GET /api/nope` | 404 |
- [ ] **Step 3: Switching back to auto** (Review Focus 5). Mode `on` with no motion for over 15 s, then `auto`:
  the lamp goes off at once with `lamp off (no motion 15 s)`. Mode `off` in the dark while waving, then `auto`:
  the lamp goes on at once with `lamp on (dark + motion)`.
- [ ] **Step 4: More than 10 events**: 12 requests alternating `on` and `off`. Expected: `events` has exactly 10
  entries, oldest first, the same as the last 10 event lines on Serial.
- [ ] **Step 5: SAVE as `03-extra-web-control`** (type extra). README notes: no login on the page (anyone on the
  network can switch the lamp), the mode is not kept across a reset, why the match is exact.

### Task 9: Finish the lesson

- [ ] **Step 1: Run `/finish-lesson`** (`.claude/skills/finish-lesson/SKILL.md`): it builds every step, checks code
  against circuits and READMEs, asks the check questions and waits for the answers. Then it moves the draft to
  the Trash, puts the last step into `EMB27.code-workspace`, updates the docs, and commits and pushes after the
  author's yes.
