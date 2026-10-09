---
name: new-lesson
description: Use when the user starts a new course lesson or homework in the EMB27 repo (a new N.M topic or a new module), runs /new-lesson, or has a plain PlatformIO project with src/main.cpp that must follow the tasks layout.
---

# New lesson

## Overview

Every course lesson with homework is one PlatformIO project at `module-N/N.M-<topic>/`.
Every homework step is a task folder with its own environment. This skill creates the skeleton
and registers it everywhere, so nothing is forgotten.

Talk to the user in Ukrainian. Everything written into the repo is English.
Do not write the homework solution: `main.cpp` is an empty skeleton (CLAUDE.md agreement).
The user invoked the whole procedure, so it counts as one step: do steps 1–4, then report and stop.

## Inputs

Take them from the arguments (`/new-lesson 1.5 button`) or the conversation.
- lesson number `N.M`;
- `<topic>`: kebab-case English folder slug. If it's missing, derive it from the lesson topic in
  `module-N/README.md` (for a new module, from https://beetroot.academy/courses/online/kurs-embedded-development)
  and say in the report that the user can rename it before the commit;
- the homework assignment. Ask for it if it isn't in the conversation. From it derive
  `<task>` = `01-base-<what-it-does>` (e.g. `01-base-button-led`) and a short name: 2–4 English words
  for what the homework builds (e.g. "Police light", "Button LED");
- the circuit: which pins, and how inputs are wired (internal pull-up, or an external pull-up/pull-down).
  The lecture decides this and it changes the code logic. Ask. If there is no answer,
  choose safe pins (see the avoid list in `module-N/CLAUDE.md`) and list every choice in the report.

## Steps

1. **Module folder.** If `module-N/` doesn't exist, create it in the shape of `module-1/`:
   - `module-N/README.md`: title, `[← All modules](../)`, one-line summary, table
     `| Lesson | Topic | Homework |` with every topic of the module from the course page, homework `—`;
   - `module-N/CLAUDE.md` with the same sections as `module-1/CLAUDE.md`: lessons → folders → status,
     hardware in this module, interview questions / what to catch in AI code (from `PLAN.md` block 2
     for this module's topics), flexible slot (from the root `CLAUDE.md` table), lesson summaries, open threads;
   - root `README.md` modules table: link `[…](module-N/)` and status `in progress`.
2. **Lesson project** `module-N/N.M-<topic>/` from the templates below: `platformio.ini`, `README.md`,
   and `tasks/<task>/` with `main.cpp`, `diagram.json` (only the parts the assignment names),
   `wokwi.toml`, `README.md`. Don't create `include/`, `lib/`, `test/` or `.gitignore`.
   If the folder already exists as a plain PlatformIO project, convert it: move `src/main.cpp`
   to `tasks/<task>/main.cpp` (`git mv` if tracked) and delete the empty `src/`.
   Other lessons stay as they are.
3. **Register it:**
   - `EMB27.code-workspace` → add `{ "path": "module-N/N.M-<topic>" }` to `folders`
     (PlatformIO only sees projects that are workspace roots);
   - `module-N/README.md` → the lesson row's homework cell: `[<short name>](N.M-<topic>/) — in progress`;
   - `module-N/CLAUDE.md` → lessons table: folder `N.M-<topic>`, status `⏳ in progress`;
     "Pins used so far" in the hardware section, if the circuit uses new pins.
4. **Verify.** Everything must pass before you report:
   ```bash
   L=module-N/N.M-<topic>
   rm -rf $L/.pio/build/<task>/src          # force our sources to recompile, so warnings show
   ~/.platformio/penv/bin/pio run -d $L -e <task> 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'
   python3 -m json.tool $L/tasks/<task>/diagram.json > /dev/null
   # Cyrillic in the repo (tracked + new files) — must print nothing:
   git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
   git status --short   # no lesson .gitignore, .pio/ or .vscode/ (the root .gitignore covers them)
   ```
5. **Report to the user** (Ukrainian):
   - what was created and registered;
   - every choice you made yourself (folder slug, short name, pins, wiring), so the user can correct it;
   - each command you ran, explained in a line, with what its output means (CLAUDE.md style);
   - where to work: the `N.M-<topic>` root in the workspace (`Developer: Reload Window` if it's not visible),
     `tasks/<task>/main.cpp`;
   - the next step, and a suggested commit message `module-N/N.M-<topic>: start lesson`.
     Commit and push only after the user approves it.

## Templates

`platformio.ini`:
```ini
; PlatformIO Project Configuration File
; https://docs.platformio.org/page/projectconf.html
;
; One environment per task: tasks/<env-name>/ holds that task's main.cpp,
; Wokwi circuit (diagram.json + wokwi.toml) and README.

[platformio]
src_dir = tasks
; The task you are working on. Build/Upload use it unless you pick another
; environment in the status bar. Without it, Upload flashes EVERY task in turn.
default_envs = <task>

[env]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
; warnings for our own code only, not for the Arduino framework
build_src_flags = -Wall -Wextra

[env:<task>]
build_src_filter = +<<task>/>
```

`tasks/<task>/main.cpp`:
```cpp
#include <Arduino.h>

void setup()
{
}

void loop()
{
}
```

`tasks/<task>/diagram.json` (board-pin names: `"47"`, `"3V3.1"`, `"GND.1"`;
part pin names: https://docs.wokwi.com/parts/<part-type>):
```json
{
  "version": 1,
  "author": "Oleksandr Bielodubrovskyi",
  "editor": "wokwi",
  "parts": [
    { "type": "board-esp32-s3-devkitc-1", "id": "esp", "top": 0, "left": 0, "attrs": {} }
  ],
  "connections": [
    [ "esp:TX", "$serialMonitor:RX", "", [] ],
    [ "esp:RX", "$serialMonitor:TX", "", [] ]
  ],
  "dependencies": {}
}
```

`tasks/<task>/wokwi.toml` (paths are relative to this file):
```toml
[wokwi]
version = 1
firmware = '../../.pio/build/<task>/firmware.bin'
elf = '../../.pio/build/<task>/firmware.elf'
```

Lesson `README.md`: copy the structure of `module-1/1.3-esp32-blink/README.md`. That means a title
`# Lesson N.M — <Topic>. Homework: <short name>`, `[← Module N](../)`, the board line, the assignment
in English, the tasks table `| # | Task | Type | What it shows |` with the `01` row, and the
"Working with the tasks" section. Task `README.md`: `# 01 — Base: <title>`, what it does,
a wiring table `| Part | GPIO |`, and **Notes** (what breaks on real hardware; no solution code).

## Common mistakes

| Mistake | Consequence |
|---|---|
| Lesson not added to the workspace `folders` | PlatformIO doesn't see the project: no Build/Upload |
| No `default_envs` | Upload flashes every task in turn; the board ends up running the last one |
| `build_flags` instead of `build_src_flags` | Arduino framework warnings flood the output |
| `wokwi.toml` points at another env | ▶ simulates the wrong task |
| Choosing pull-up vs pull-down silently | The user's code logic won't match the lecture's circuit |
| Warning check on a cached build | Unchanged sources aren't recompiled, so warnings never show |
| `grep -P` for the Cyrillic check | Stock macOS grep has no `-P`; use the perl line above |
| Writing the homework solution | The user writes it or spots mistakes in it; that's the point of the course |
| Committing the lesson `.gitignore` | PlatformIO regenerates it; the root ignores `/module-*/*/.gitignore` |
