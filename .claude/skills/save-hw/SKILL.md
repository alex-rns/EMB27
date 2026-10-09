---
name: save-hw
description: Use when the user wants to save a course homework in the EMB27 repo ("save the homework", /save-hw), usually pasting the assignment, while the code still lives in a messy PlatformIO project from class.
---

# Save homework

## Overview

The user experiments in an ordinary PlatformIO project with no rules: one `src/main.cpp`, copies,
"archive" folders. This skill turns the result into the saved structure: a lesson folder with one
folder per homework step, and **each step is an ordinary PlatformIO project**.
The user must only ever see standard projects: never multiple environments, `src_dir` or `build_src_filter`.

Talk to the user in their language (see their CLAUDE.md). Everything written into the repo is English.
Move the user's code as it is. Don't fix bugs in it: spotting them is the user's exercise.

## Inputs

- **The assignment** (ask for it if it's not in the conversation). From it:
  - the lesson number `N.M` (or ask);
  - the steps: the required part → `01-base-<slug>`, each optional item → `02-extra-<slug>`, `03-extra-…`;
    a class demo → `00-lecture-<slug>`, only if the user wants to keep it. Slugs: kebab-case English.
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
2. **Map code to steps.** Read everything in the Trash copy: `src/main.cpp`, copies, archive folders,
   other files, plus our conversation. For each step pick the version of the code that implements it.
   If a step has no version, ask. If the user isn't available, give it an empty skeleton and say so.
3. **Build the lesson folder** `module-N/N.M-<topic>/` (`<topic>` comes from the lesson topic in
   `module-N/README.md`). For each step create `NN-<type>-<slug>/`:
   - `platformio.ini`, `wokwi.toml`: the templates below;
   - `src/main.cpp`: the mapped version, unchanged;
   - `diagram.json`: a Wokwi circuit matching the pins used in that step's code;
   - `README.md`: `# NN — <Type>: <title>`, what the step does, a `| Part | GPIO |` wiring table,
     and **Notes** on what breaks on real hardware.

   Then add the lesson `README.md` in the shape of `module-1/1.3-esp32-blink/README.md`: the assignment
   in English, a steps table, and "Working with the steps".
4. **Register:**
   - `EMB27.code-workspace`: remove the source project's entry, because it's gone. Don't add the steps;
   - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — base task + K extras`;
   - `module-N/CLAUDE.md`: lessons table → folder + `✅ saved · ⏳ /finish-lesson`; "Pins used so far";
   - a new module gets `module-N/README.md` and `module-N/CLAUDE.md` shaped like `module-1/`,
     plus its row in the root `README.md`.
5. **Verify.** Every check must pass:
   ```bash
   L=module-N/N.M-<topic>
   for s in $L/*/; do echo "== $s"; ~/.platformio/penv/bin/pio run -d "$s" 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'; done
   for s in $L/*/; do python3 -m json.tool "$s/diagram.json" > /dev/null || echo "BAD JSON $s"; done
   # Cyrillic in the repo (tracked + new files) — must print nothing:
   git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
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
| `rm` instead of the Trash | The user's experiments are gone for good |
| Fixing the user's code while moving it | The saved step isn't what they wrote, and they lose the exercise |
| `diagram.json` pins differ from the code | Wokwi simulates a different circuit |
| Leaving the source project in the workspace | VS Code shows a missing folder |
| Committing before the user's yes | Breaks the CLAUDE.md agreement on commits |
