---
name: save-hw
description: Use when the user wants to save a course homework in the EMB27 repo ("save the homework", /save-hw), usually pasting the assignment, while the code still lives in a messy PlatformIO project from class.
---

# Save homework

## Overview

The user works in the lesson's draft, an ordinary PlatformIO project: one `src/main.cpp`, and each
finished piece copied to `archive/<type>-<slug>.cpp` (outside `src/`, where every `.cpp` is compiled).
This skill turns the result into the saved structure: a lesson folder with one folder per step, and
**each step is an ordinary PlatformIO project**.
The user must only ever see standard projects: never multiple environments, `src_dir` or `build_src_filter`.

Talk to the user in their language (see their CLAUDE.md). Everything written into the repo is English.
Move the user's code as it is. Don't fix bugs in it: spotting them is the user's exercise.

## Inputs

- **The assignment** (ask for it if it's not in the conversation). From it:
  - the lesson number `N.M` (or ask);
  - the homework steps: the required part → `base`, each optional item → `extra`.
- **The steps**, numbered from `00` in the order the work happened: `practice` → `lecture` → `base` → `extra`.
  - own practice before the lecture (`archive/practice-*.cpp`) → `NN-practice-<slug>`, one step per file,
    always kept. It is optional: a lesson without it starts at the lecture or the base task;
  - a class demo (`archive/lecture-*.cpp`) → `NN-lecture-<slug>`, only if the user wants to keep it;
  - then `NN-base-<slug>` and `NN-extra-<slug>`. Slugs: kebab-case English.

  Examples: lecture + base + 4 extras → `00`–`05` (lesson 1.3); practice + lecture + base → `00-practice`,
  `01-lecture`, `02-base`.
- **The source project**: the folder with a `platformio.ini` the user worked in. Check the workspace folders,
  `module-N/`, and `git status`. If there are several candidates, ask which one.

## Steps

1. **Move the source project to the Trash.** The Trash keeps everything, and this frees the path
   if the source sits where the lesson folder must go:
   ```bash
   T=~/.Trash/<project-name>-$(date +%Y%m%d-%H%M)
   git rm -r -q --cached <source> 2>/dev/null   # only if it was tracked; history keeps it
   mkdir -p "$T" && mv <source> "$T"/
   ```
   Never `rm` the user's work.
2. **Map code to steps.** Read everything in the Trash copy: `archive/` first (one file per finished
   piece, named by its type), then `src/main.cpp`, other copies and files, plus our conversation.
   For each step pick the version of the code that implements it.
   If a step has no version, ask. If the user isn't available, give it an empty skeleton and say so.
3. **Build the lesson folder** `module-N/N.M-<topic>/` (`<topic>` comes from the lesson topic in
   `module-N/README.md`). For each step create `NN-<type>-<slug>/`:
   - `platformio.ini`, `wokwi.toml`: the templates below;
   - `src/main.cpp`: the mapped version, unchanged;
   - `diagram.json`: a Wokwi circuit matching the pins used in that step's code. Wokwi closes an
     unrouted wire horizontally first, then vertically, so a wire with `[]` easily runs over a row of
     pins or along another wire, and the picture shows connections that don't exist. Give wires
     placement instructions (`"h-30"`, `"*"`; see the 1.3 steps) so that no wire crosses the board or
     a pin, and wires of different nets never overlap. Wokwi's router (read in its editor code):
     after `"*"` it closes the gap only when both x and y differ, otherwise it silently drops the last
     segment to the target pin; after a `"v…"` instruction it closes the gap vertically first.
     Check every wire's path by code with these rules before asking for screenshots;
   - `diagram.png`: ask the user for a screenshot of the circuit from the Wokwi editor (`Cmd+Shift+4`),
     check it shows the same wiring as `diagram.json`, and save it next to it;
   - `schematic.svg`: the step's real circuit (not the Wokwi stand-ins) as a schematic drawn with schemdraw
     (the script stays outside the repo). Render it to PNG and look at it before saving: no label on a line or
     a part, every part and net as in the code and the README. A step with only LEDs on GPIOs may skip it;
   - `README.md`: `# NN — <Type>: <title>`, what the step does, a `| Part | GPIO |` wiring table,
     `![Schematic](schematic.svg)`, `![Circuit in Wokwi](diagram.png)`, and **Notes** on what breaks on real hardware. A `practice` step
     also says what was checked and the result (e.g. which GPIO drives which onboard LED).

   Then add the lesson `README.md` in the shape of `module-1/1.3-esp32-blink/README.md`: the assignment
   in English, a steps table, and "Working with the steps".
4. **Register:**
   - `EMB27.code-workspace`: replace the source project's entry with the lesson's last step, so one
     PlatformIO project stays open (without one the PlatformIO buttons disappear). Add no other steps;
   - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — base task + K extras`,
     plus ` + P practice` when the lesson has practice steps;
   - `module-N/CLAUDE.md`: lessons table → folder + `✅ saved · ⏳ /finish-lesson`; "Pins used so far";
   - a new module gets `module-N/README.md` and `module-N/CLAUDE.md` shaped like `module-1/`,
     plus its row in the root `README.md`.
5. **Verify.** Every check must pass:
   ```bash
   L=module-N/N.M-<topic>
   for s in $L/*/; do echo "== $s"; ~/.platformio/penv/bin/pio run -d "$s" 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'; done
   for s in $L/*/; do python3 -m json.tool "$s/diagram.json" > /dev/null || echo "BAD JSON $s"; done
   # Cyrillic in the repo (tracked + new files) — must print nothing:
   git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'BEGIN { @ARGV = grep { -T } @ARGV } print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
   git status --short   # no .pio/, .vscode/ or project .gitignore
   ```
   A compiler warning in the user's code is a learning point: explain it, but don't fix it unasked.
6. **Report to the user**:
   - a table "step → which version of your code it came from";
   - the choices you made yourself;
   - what went to the Trash, and where;
   - the build results, each command explained in a line;
   - `git status --short`.

   Commit only after the user says yes: `module-N/N.M-<topic>: save homework`, then `git push`.
   Then offer `/finish-lesson` for the check questions.

## Templates

`platformio.ini` (the wizard's file plus warnings for our own code):
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_src_flags = -Wall -Wextra
```

`wokwi.toml`:
```toml
[wokwi]
version = 1
firmware = '.pio/build/esp32-s3-devkitc-1/firmware.bin'
elf = '.pio/build/esp32-s3-devkitc-1/firmware.elf'
```

`diagram.json` starting point. Board-pin names are `"47"`, `"3V3.1"`, `"GND.1"`. Parts: `wokwi-led`
(pins `A`, `C`), `wokwi-resistor` (`1`, `2`); for others see https://docs.wokwi.com/parts/<part-type>.
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

## Common mistakes

| Mistake | Consequence |
|---|---|
| Multiple environments, `src_dir`, `build_src_filter` | The user explicitly finds them confusing |
| `01` hard-coded for the base task | Wrong order: numbers follow the work (practice → lecture → base → extra) |
| `rm` instead of the Trash | The user's experiments are gone for good |
| Fixing the user's code while moving it | The saved step isn't what they wrote, and they lose the exercise |
| `diagram.json` pins differ from the code | Wokwi simulates a different circuit |
| Wires left as `[]` | The picture shows wires over pins or merged: someone wiring from it makes a short |
| Leaving the source project in the workspace | VS Code shows a missing folder |
| Removing it without putting the last step in its place | No PlatformIO project left: the buttons vanish after a reload |
| Committing before the user's yes | Breaks the CLAUDE.md agreement on commits |
