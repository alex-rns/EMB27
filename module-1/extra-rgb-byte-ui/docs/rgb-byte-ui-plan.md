# RGB byte UI Implementation Plan

> **Executed interactively, not by agents.** The author writes the first working version of each step by hand from
> the hints below; the assistant then reviews it and improves it in small, explained changes. The hints say what a
> step must do, what to wire, which functions or notes to look at and in what order. They never prescribe the code's
> structure (variables, data structures, functions). Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the RGB byte UI from the design in eight small steps, each a working program on the real board
and one commit.

**Approach:** One ordinary PlatformIO project, `module-1/extra-rgb-byte-ui/`, grows step by step. When a step works
on the board and has been reviewed, it is committed: one commit per step, so the git history shows the progress
(the author's decision of 2026-10-20; no step folders, no Wokwi circuit per step). The breadboard grows with the
steps: nothing is wired before the step that needs it.

**Tech Stack:** ESP32-S3 DevKitC-1 clone (N16R8), PlatformIO board `esp32-s3-devkitc-1`, Arduino core 2.0.17;
breadboard, 3 mm LEDs, tactile buttons, a discrete RGB LED, a multimeter.

**Spec:** [`rgb-byte-ui-design.md`](rgb-byte-ui-design.md)

## Global Constraints

- Arduino core **2.0.17**: check every API against the installed core
  (`~/.platformio/packages/framework-arduinoespressif32/`), never against 3.x docs.
- Every project: the wizard's `[env:esp32-s3-devkitc-1]` (`platform = espressif32`, `board = esp32-s3-devkitc-1`,
  `framework = arduino`) plus `build_src_flags = -Wall -Wextra` and `monitor_speed = 115200`. One environment, no
  `src_dir`, no `build_src_filter`, no `lib_deps`. A build has no `warning:` line from our code.
- Pins only from 1, 2, 4–18, 21, 38–42. Never 0, 3, 45, 46, 19, 20, 26–37, 43, 44; leave 47 and 48 unused.
- Every LED has a 220 Ω resistor in series. Buttons go between the GPIO and GND, read with the internal pull-up.
- The bit 0 LED is the rightmost one on the breadboard, bit 7 the leftmost.
- Everything in the repo is in English, comments included.
- Who does what: the author picks the pins, does every hands-on action (wiring, Upload, Serial Monitor, multimeter,
  pressing buttons), one action at a time, and writes each step's first version. The assistant checks the chosen pins,
  reviews and improves the code, builds and commits the step.
- A commit and push only after the author's yes. Message: `module-1/extra-rgb-byte-ui: <what was done>`.

## How a step is verified (no unit tests)

There is no host test environment (one PlatformIO environment per project). Each step's **board checks** are written
below before any code; they fail first because the project doesn't do it yet. A step is done when every board check
passes on the real board and the build has no warnings.

## The cycle of every step (CYCLE)

1. **Hints:** the assistant goes through the step's hints with the author (in Ukrainian), no code.
2. **Wiring:** the author picks pins for the new parts; the assistant checks them against the constraints; the author
   wires them, one part at a time.
3. **First version:** the author writes it in the project's `src/main.cpp`, builds, uploads and runs the board checks.
4. **Review:** the assistant reads the author's version, explains what works and what is weak or will break on the
   board, then improves it in small changes, each explained; the author re-runs the board checks after each change.
5. **COMMIT** (below), then the next step.

## Commit procedure (COMMIT)

1. Build (`~/.platformio/penv/bin/pio run -d module-1/extra-rgb-byte-ui`: `SUCCESS`, no `warning:`), run
   `.github/scripts/checks.sh`, check `git status --short`: only this project's files (no `.pio/`, no `.vscode/`).
2. First commit only: a row in `module-1/README.md` and in `module-1/CLAUDE.md` (lessons table, pins used).
3. Suggest `module-1/extra-rgb-byte-ui: step NN, <what the step does>`; commit and push after the author's yes.
4. After the last step: the project `README.md` (what it does, the wiring table, what breaks on real hardware).

## Review Focus

1. **A held button** (half a second, two seconds) acts once. Board check in Task 2.
2. **A mirrored row**: the value 1 must light only the rightmost LED, 128 only the leftmost. Board check in Task 1.
3. **Release bounce** never adds a step: twenty slow presses give exactly twenty. Board check in Task 2.
4. **A common-anode RGB LED is lit at power-on** (inverted duty): it must stay dark until button 6. Board check in
   Task 6.
5. **Two buttons pressed together** (3 and 4): exactly one indicator ends up lit. Board check in Task 4.

---

### Task 0: The project

- [ ] **Step 1:** the assistant explains, then runs
  `~/.platformio/penv/bin/pio project init -d module-1/extra-rgb-byte-ui -b esp32-s3-devkitc-1 -O framework=arduino`
  and adds the two constraint lines to `platformio.ini`.
- [ ] **Step 2:** in `EMB27.code-workspace`, replace the `1.7 / 02-extra-web` folder with
  `{"name": "RGB byte UI", "path": "module-1/extra-rgb-byte-ui"}`; the author reopens the workspace and sees the
  PlatformIO buttons.
- [ ] **Step 3:** no build yet: `pio project init` creates an empty `src/`, and the Arduino framework doesn't link
  without `setup()` and `loop()`. The first build comes with the author's `src/main.cpp` in Task 1. The wizard's
  placeholder `README` files in `include/`, `lib/` and `test/` are removed (no committed project keeps them).

### Task 1: Step `00-base-byte-leds` — the eight LEDs show a number

**Does:** the eight LEDs show a byte written in the code; bit 7 leftmost.
**Wire:** 8 LEDs of one colour, each GPIO → 220 Ω → LED → GND (long leg towards the GPIO).
**Look at:** `pinMode`, `digitalWrite` (lesson 1.3); "test a bit" from the bits lesson (the result is the bit's
weight, not 1); `uint8_t`.
**Order:** pick 8 pins; wire and blink one LED first to prove the direction; wire the rest; then the code.

- [ ] Board checks: `160` → `●○●○○○○○`; `1` → only the rightmost; `128` → only the leftmost; `255` → all;
  `0` → none; `85` → `○●○●○●○●`. Each value is a re-flash.
- [ ] CYCLE, COMMIT.

### Task 2: Step `01-base-cursor` — button 1 moves the cursor

**Does:** one lit LED (the cursor), nothing else lit; it starts on bit 0; each press of button 1 moves it one place
left, from bit 7 back to bit 0; one press is one step, however long the button is held.
**Wire:** button 1.
**Look at:** `INPUT_PULLUP` (lesson 1.4: pressed reads LOW); `millis()` and the `now - last >= period` form (lesson
1.5); `Serial.begin`, `Serial.println`.
**Order:** checkpoint first: one Serial line per press, with a running count, and nothing else; then the cursor.

- [ ] Checkpoint: 20 slow presses → the count ends at 20; no line on any release; holding the button 2 s → one line.
- [ ] Board checks: 8 presses from power-on → back on bit 0; a 9th → bit 1; holding 2 s → one step.
- [ ] CYCLE, COMMIT.

### Task 3: Step `02-base-toggle-bit` — button 2 toggles the bit under the cursor

**Does:** button 2 toggles the bit under the cursor; the LEDs show the byte, and the cursor's LED is lit whatever
its bit.
**Wire:** button 2.
**Look at:** "toggle a bit" from the bits lesson; the drawing rule we worked out (cursor first, then the bit).
Optional for debugging: print the byte after each press, `Serial.println(value, BIN)`.

- [ ] Board checks: from power-on, 1 ×5, 2, then 1 → bit 5 stays lit after the cursor leaves; then 1 ×1, 2, 1 ×1
  → `●○●○○○○●`, where the rightmost is the cursor on bit 0 (the byte is 160); 2 twice on the same bit → no change;
  holding 2 → one toggle.
- [ ] CYCLE, COMMIT.

### Task 4: Step `03-base-select` — buttons 3/4/5 and the indicators

**Does:** buttons 3/4/5 select R/G/B; only the selected indicator is lit; R is selected at power-on; there is still
one byte, and the eight LEDs don't change on a selection.
**Wire:** buttons 3–5; a red, a green and a blue LED with 220 Ω next to them.
**Look at:** nothing new; the indicator rule we worked out.

- [ ] Board checks: power-on → only red; 4 → only green; 5 → only blue; 3 → only red; 4 twice → still green;
  buttons 1 and 2 still work; 3 and 4 pressed together → exactly one indicator lit.
- [ ] CYCLE, COMMIT.

### Task 5: Step `04-base-slots` — each slot keeps its own byte

**Does:** button 2 changes the selected slot's byte; selecting a slot shows its byte; the cursor stays where it is;
all slots are 0 at power-on.
**Wire:** nothing.

- [ ] Board checks: set R = 160 (bits 7 and 5); 4 → only the cursor's LED lit (G = 0); move the cursor to bit 6,
  press 2 → G = 64; 3 → bits 7 and 5 lit plus the cursor's LED on bit 6; 4 → only bit 6 lit (the cursor is on it
  too); RST → all slots 0, R selected.
- [ ] CYCLE, COMMIT.

### Task 6: Step `05-base-rgb` — button 6 and the RGB LED

**Does:** button 6 lights the RGB LED with the three slots; the RGB LED is dark at power-on; later edits don't reach
it until 6 is pressed again.
**Before wiring:** the multimeter's diode test on the RGB LED: which leg is common, anode or cathode, which leg is R,
G and B (the author; the result goes into the step README).
**Wire:** R, G, B legs each through 220 Ω to a GPIO; the common leg to GND (cathode) or 3V3 (anode); button 6.
**Look at:** `ledcSetup`, `ledcAttachPin`, `ledcWrite` in the installed core's `esp32-hal-ledc.h`; 8-bit resolution;
the lesson 1.3 note that `pinMode` after `ledcAttachPin` detaches the PWM.
**Order:** checkpoint first: fixed colours from the code, red, then green, then blue; then button 6.

- [ ] Checkpoint: each fixed colour lights only its own die.
- [ ] Board checks: power-on → dark; only R = 255, 6 → red; only G = 255 → green; only B = 255 → blue; all 0, 6 →
  dark; change R after 6 → no change until 6 again.
- [ ] CYCLE, COMMIT.

### Task 7: Step `06-extra-blink` — the cursor blinks

**Does:** the cursor's LED blinks at a fixed rate, the same over a 0 and a 1; no press is lost.
**Look at:** `millis()`; why `delay()` in `loop()` misses presses (lesson 1.4).

- [ ] Board checks: the cursor blinks over a 0 and over a 1; 10 quick presses of 1 → exactly 10 steps; holding 1 →
  one step; the other seven LEDs never blink.
- [ ] CYCLE, COMMIT.

### Task 8 (optional): Step `07-extra-remember` — the slots survive a power cycle

- [ ] A short design talk first (what is written to flash and when, what comes back at boot), recorded in the design.
- [ ] Hints then point at the built-in `Preferences` library of core 2.0.17; board checks come from that talk and run
  on the board only (a real reset and a real power cycle).
- [ ] CYCLE, COMMIT.

### Task 9: Finish

- [ ] The project `README.md`, then `/finish-lesson` adapted to one project: build it, check the code against the
  README, check questions, docs, commit.
