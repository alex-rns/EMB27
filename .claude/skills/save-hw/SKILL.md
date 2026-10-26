---
name: save-hw
description: Use when the user wants to save a course homework in the EMB27 repo ("save the homework", /save-hw), usually pasting the assignment, while the code still lives in a messy PlatformIO project from class.
---

# Save homework

## Overview

The user works in the lesson's draft, an ordinary PlatformIO project: one `src/main.cpp`, and each
finished piece copied to `archive/<type>-<slug>.cpp` (outside `src/`, where every `.cpp` is compiled), or, when
it has headers of its own, to a folder `archive/<type>-<slug>/` with `main.cpp` and those headers.
The homework is written in the same draft after the practice and the lecture, **one commit per change**.
This skill turns the result into the saved structure: a lesson folder with one folder per step, and
**each step is an ordinary PlatformIO project**. The practice and lecture pieces become steps on the lesson branch;
the homework becomes **one folder** with the final code of every task, rebuilt on the homework branch commit by commit
from the draft's homework commits, so the teacher sees the final code and can walk the steps in the pull request.
The user must only ever see standard projects: never multiple environments, `src_dir` or `build_src_filter`.

Talk to the user in their language (see their CLAUDE.md). Everything written into the repo is English.
Move the user's code as it is. Don't fix bugs in it: spotting them is the user's exercise.

## Inputs

- **The assignment** (ask for it if it's not in the conversation): the lesson number `N.M` (or ask) and its full
  text, which goes translated into the homework README and the pull request.
- **The steps**, numbered from `00` in the order the work happened: `practice` → `lecture` → `homework`.
  - own practice before the lecture (`archive/practice-*.cpp` or `archive/practice-*/`) → `NN-practice-<slug>`,
    one step per file or folder, always kept, even when its code matches the homework. It is optional;
  - a class demo (`archive/lecture-*.cpp` or `archive/lecture-*/`) → `NN-lecture-<slug>`, only if the user wants to keep it;
  - the homework → one `NN-homework-<slug>`: every point of the assignment, required and optional, in one working
    project. Slugs: kebab-case English.
  - from module 2 on every step name ends with its board (root `CLAUDE.md`, "Two boards"): this skill makes the
    `-esp32` steps, e.g. `01-homework-blink-esp32`. An STM32 version is a CubeIDE project CubeMX generates straight
    into its own `NN-<type>-<slug>-stm32` folder, with the same number and slug as the ESP32 step it repeats; this
    skill only lists it in the lesson README.

  Example: practice + homework → `00-practice-blink-esp32`, `01-homework-blink-esp32` (lesson 2.1). Module 1 split
  the homework into `base` and `extra` steps; it stays as it is.
- **The homework commits**: on the lesson branch, the draft's commits from the homework's starting point to its last
  change (`git log --format='%h %aI %s' -- module-N/N.M-<topic>`). Archive commits aren't homework steps.
- **The source project**: the folder with a `platformio.ini` the user worked in. Check the workspace folders,
  `module-N/`, and `git status`. If there are several candidates, ask which one.
- **Circuits:** ask once whether the user wants Wokwi circuits and schematics for this lesson. Without a yes, the
  steps get none (`diagram.json`, `wokwi.toml`, `diagram.png`, `schematic.svg`, `wokwi-scenario.yaml`); the README's
  wiring table is enough.

## Steps

1. **Move the source project to the Trash.** Ask the user to close the draft's files in VS Code first: with a
   draft file still open, the PlatformIO extension was seen re-creating an empty project in the freed folder
   (`platformio.ini` with comments only, `include/`, `lib/`, `test/`, `src/`, `.vscode/`; 2026-10-26).
   The Trash keeps everything, and this frees the path if the source sits where the lesson folder must go:
   ```bash
   T=~/.Trash/<project-name>-$(date +%Y%m%d-%H%M)
   git rm -r -q --cached <source> 2>/dev/null   # only if it was tracked; history keeps it
   mkdir -p "$T" && mv <source> "$T"/
   ```
   Never `rm` the user's work.
   The draft is usually the lesson folder itself. If it already holds `*-stm32` step folders (CubeIDE projects, not
   part of the draft), move them aside first (`mv` next to the lesson folder) and back into the new lesson folder in
   step 3, so they don't go to the Trash with the draft.
2. **Map code to steps.** Read everything in the Trash copy: `archive/` first (one file or folder per finished
   piece, named by its type), then `src/`, `include/`, other copies and files, plus our conversation.
   Practice and lecture steps take their `archive/` version. The homework takes the draft's homework commits:
   list them with the files each one changed (`git show --stat`); the last one is the homework's final code.
   If a step has no version, ask.
3. **Build the lesson folder** `module-N/N.M-<topic>/` (`<topic>` comes from the lesson topic in
   `module-N/README.md`). For each practice or lecture step create `NN-<type>-<slug>/` (`…-esp32/` from module 2 on):
   - `platformio.ini`: the template below, plus `monitor_speed = 115200` when the code uses `Serial`;
   - `src/main.cpp`: the mapped version, unchanged; from an `archive/<type>-<slug>/` folder, `main.cpp` goes to
     `src/` and the headers to `include/`, all unchanged;
   - `README.md`: `# NN — <Type>: <title>`, what the step does, a `| Part | GPIO |` wiring table, and **Notes** on
     what breaks on real hardware. A `practice` step also says what was checked and the result;
   - circuits only if the user asked for them (see "Circuits" below).

   The homework folder is **not** created here: the homework branch builds it (step 6). Prepare its files in the
   session scratchpad instead:
   - `README.md`: `# NN — Homework: <title>`, the assignment translated in full (every point, the format and the
     concepts lists too), a table "point → where in the code", wiring, what was checked on the board (with the
     numbers), **Notes** on what breaks on real hardware;
   - the replay script (template below), with one `step` line per homework commit and the `platformio.ini` lines
     each commit needs (e.g. `monitor_speed` together with the commit that adds `Serial`).

   Then add the lesson `README.md`: the lesson title, a short line on what it covers, a steps table, and "Working
   with the steps". On the lesson branch it lists the practice and lecture steps only; the homework's row and a link
   to the assignment in the homework README come with the homework branch.
4. **Register:**
   - `EMB27.code-workspace`: replace the source project's entry with the homework folder, so one PlatformIO project
     stays open (without one the PlatformIO buttons disappear). Add no other steps;
   - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — homework`, plus ` + P practice` /
     ` + L lecture` when the lesson has those steps;
   - `module-N/CLAUDE.md`: lessons table → folder + `✅ saved, PR open`; "Pins used so far";
   - a new module gets `module-N/README.md` and `module-N/CLAUDE.md` shaped like `module-2/`,
     plus its row in the root `README.md`.
5. **Verify.** Every check must pass:
   ```bash
   L=module-N/N.M-<topic>
   for s in $L/*/; do echo "== $s"; ~/.platformio/penv/bin/pio run -d "$s" 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'; done
   # Cyrillic in the repo (tracked + new files) — must print nothing:
   git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'BEGIN { @ARGV = grep { -T } @ARGV } print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
   git status --short   # no .pio/, .vscode/ or project .gitignore
   ```
   Test the replay script in a throwaway worktree, never on a real branch: `git worktree add --detach <scratch>/wt HEAD`,
   run the script there, check that every commit built and that the final folder matches the homework's final code
   (`diff -q`), then `git worktree remove --force <scratch>/wt`.
   A compiler warning in the user's code is a learning point: explain it, but don't fix it unasked.
6. **Report to the user**:
   - a table "step → which version of your code it came from", and the homework commits the replay will make;
   - the choices you made yourself;
   - what went to the Trash, and where;
   - the build results, each command explained in a line;
   - `git status --short`.

   Commit only after the user says yes, in two parts (CLAUDE.md, Git), so the pull request the teacher reviews
   holds nothing but the homework:
   1. **The lesson branch** `module-N/N.M-<topic>`: the draft's removal, the `practice` and `lecture` steps, the
      lesson README without the homework row, `module-N/README.md`, `module-N/CLAUDE.md`, `EMB27.code-workspace`.
      Commit `module-N/N.M-<topic>: save the practice and lecture steps` (name only the kinds it has), push, open a
      pull request (`gh pr create --base main`, title `module-N/N.M-<topic>`) and merge it after the user's yes
      without waiting for CI (`gh pr merge --merge --delete-branch`). If the lesson branch has nothing left to
      commit, skip this part.
   2. **The homework branch** `module-N/N.M-<topic>-homework`, from the updated `main`
      (`git switch main && git pull && git switch -c module-N/N.M-<topic>-homework`): run the replay script, then copy
      the homework README in, add the homework's row and the assignment link to the lesson README, and commit
      `module-N/N.M-<topic>: homework, describe the assignment, how each point is done and the board results`.
      Push with `git push -u origin module-N/N.M-<topic>-homework`. Check that the branch holds nothing else:
      after `git fetch origin`, `git diff --name-only origin/main...HEAD` lists only the homework folder and the
      lesson README, and `git log --oneline origin/main..HEAD` only the replayed commits and the README commit.
   3. **Open the homework's pull request right away**, without waiting for CI or for `/finish-lesson`:
      `gh pr create --base main --head module-N/N.M-<topic>-homework`, title `module-N/N.M-<topic>: homework`, an
      English body for the teacher (write it to a file, `--body-file`): the folder and how its history goes, the
      assignment in full (every point of the text the user pasted, translated), a table of the commits (short hash,
      change, task number), how it was tested (builds, the board, with numbers), known limits, and the attribution
      line. Never pass `--reviewer`: the user adds the teacher. `gh pr view <n> --json files --jq '.files[].path'`
      must list only the homework folder and the lesson README. Give the user the link to send to the teacher.

   Never commit lesson work to `main` directly. `/finish-lesson` (check questions, doc updates) is optional: mention
   it in one line, run it only when the user asks.

## Circuits (only when the user asked for them)

- `diagram.json` + `wokwi.toml`: a Wokwi circuit matching the pins used in that step's code. Wokwi closes an
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
  a part, every part and net as in the code and the README;
- `wokwi-scenario.yaml`: a short Wokwi CI scenario (`set-control`, `wait-serial`, `expect-pin` on GPIO-driven pins
  only; pins behind relay contacts read randomly). Run it locally from the step folder with
  `wokwi-cli . --scenario wokwi-scenario.yaml` and once with a wrong expectation to see it fail.
  Keep it to a few seconds of simulation: Wokwi minutes are limited, so it is not run in CI;
- the README then shows `![Schematic](schematic.svg)` and `![Circuit in Wokwi](diagram.png)`.

## Templates

`platformio.ini` (the wizard's file plus warnings for our own code):
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_src_flags = -Wall -Wextra
```

Replay script (lesson 2.1's, trimmed). Run from the repo root on the homework branch; it stops at the first failed
build, warning or commit:
```bash
#!/bin/bash
set -euo pipefail
SRC=module-2/2.1-embedded-cpp                      # the draft's path in the old commits
HW=$SRC/01-homework-blink-esp32
PIO=~/.platformio/penv/bin/pio
HERE=$(cd "$(dirname "$0")" && pwd)
TRAILER='Co-Authored-By: <the attribution line from the conversation>'
step() { # $1 draft commit, $2 message, rest: files to take from that commit (paths inside the draft)
  local c=$1 msg=$2; shift 2
  for f in "$@"; do mkdir -p "$HW/$(dirname "$f")"; git show "$c:$SRC/$f" > "$HW/$f"; done
  "$PIO" run -d "$HW" > "$HERE/build-$c.log" 2>&1 || { echo "BUILD FAILED at $c"; exit 1; }
  if grep -q "warning:" "$HERE/build-$c.log"; then echo "WARNING at $c"; exit 1; fi
  git add "$HW"
  GIT_AUTHOR_DATE=$(git log -1 --format=%aI "$c") git commit -q -m "$SRC: $msg" -m "$TRAILER"
  echo "$(git log -1 --format='%h %s')"
}
mkdir -p "$HW"
printf '[env:esp32-s3-devkitc-1]\nplatform = espressif32\nboard = esp32-s3-devkitc-1\nframework = arduino\nbuild_src_flags = -Wall -Wextra\n' > "$HW/platformio.ini"
step cac1829 "homework, start from the classic Arduino Blink on GPIO 7" src/main.cpp
step cc27a3d "homework, split Config and Led into include/config.h and include/led.h" src/main.cpp include/config.h include/led.h
printf 'monitor_speed = 115200\n' >> "$HW/platformio.ini"
step e7bfee6 "homework, measure the average loop() time and report it every 1000 iterations" src/main.cpp include/config.h
```

## Common mistakes

| Mistake | Consequence |
|---|---|
| Multiple environments, `src_dir`, `build_src_filter` | The user explicitly finds them confusing |
| The homework split into `base` / `extra` step folders | The user wants one working homework folder per board (from module 2 on) |
| The homework folder committed in one go | The teacher can't walk the steps the user committed one by one |
| Circuits or schematics made without asking | Work the user didn't want (lesson 2.1: they asked to remove them) |
| Replay run on a real branch before a test run | A broken replay leaves half-made commits in a published branch |
| Fixed numbers for the steps | Wrong order: numbers follow the work (practice → lecture → homework) |
| `rm` instead of the Trash | The user's experiments are gone for good |
| Fixing the user's code while moving it | The saved step isn't what they wrote, and they lose the exercise |
| Leaving the source project in the workspace | VS Code shows a missing folder |
| Homework committed on the lesson branch | The teacher's pull request also shows the draft, practice and lecture commits |
| Removing it without putting the homework folder in its place | No PlatformIO project left: the buttons vanish after a reload |
| Committing before the user's yes | Breaks the CLAUDE.md agreement on commits |
| The pull request left for `/finish-lesson` | The user wants it open right after saving (lesson 2.1) |
