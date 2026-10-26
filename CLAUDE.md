# EMB27 — project rules

> Rules for AI assistants working in this repository: homework for the Beetroot Academy
> "Embedded Development" course, group EMB27. Module-specific technical notes live in `module-N/CLAUDE.md`.

## Language

Everything in the repository is in English: code, comments, docs, configs, commit messages.
Personal notes about the author never go into this repository; it is public.

## Workflow

- Each lesson's ESP32 work is done in a draft: an ordinary PlatformIO project created by the wizard, one
  `src/main.cpp`, in the lesson's folder (`module-N/N.M-<topic-slug>/`). A finished piece (practice before the lecture, the lecture demo, …) is copied to
  `archive/<type>-<slug>.cpp` next to `src/`, never inside it: every `.cpp` in `src/` is compiled, and two
  `setup()` don't link. A piece with headers of its own goes to a folder, `archive/<type>-<slug>/`, holding
  `main.cpp` and the headers from `include/`. Otherwise no rules while experimenting.
- The homework is written in the same draft after the practice and the lecture, one commit per change.
- `/save-hw` + the assignment turns the draft into the saved structure below: a step for each practice or lecture
  piece in `archive/`, and **one homework folder** with the final code of every task, required and optional. The
  homework branch rebuilds that folder commit by commit from the draft's homework commits, so the teacher sees the
  final code and can walk the steps. READMEs always; Wokwi circuits and schematics only when the author asks for them.
  The draft goes to the macOS Trash. Commit and push after the author confirms: the lesson branch first, then the
  homework branch, and `/save-hw` opens the homework's pull request for the teacher right away (see Git).
- `/finish-lesson` is optional and runs whenever the author asks for it, before or after the save and the pull
  request: it builds every step, checks code against READMEs (and circuits, if any), asks check questions, then
  updates the docs. Nothing waits for it.
- Project skills live in `.claude/skills/` (tracked). Follow a skill when it is invoked or the situation matches it.

## Repository layout

The layout mirrors the course: module (0–7) → lesson (N.M) → steps (practice, lecture demo, homework).
```
README.md                    ← course, board, table of modules (GitHub front page)
CLAUDE.md                    ← this file
module-N/
├── README.md                ← module topics + table of lessons with links
├── CLAUDE.md                ← technical notes for this module: lessons → folders → status,
│                              hardware/pins, what to watch for, known issues
└── N.M-<topic-slug>/        ← one folder per lesson with homework (a container, not a project)
    ├── README.md            ← table of steps (type, what it shows)
    └── NN-<type>-<slug>/    ← one step = one ordinary PlatformIO project; type = practice | lecture | homework,
        │                      numbered from 00 in the order of the work (practice and lecture are optional); from
        │                      module 2 on the name ends with the board, -esp32 or -stm32 (see "Two boards")
        ├── platformio.ini   ← the wizard's 4 lines + build_src_flags = -Wall -Wextra
        ├── src/main.cpp, include/*.h
        ├── diagram.json, wokwi.toml ← Wokwi circuit (→ .pio/build/esp32-s3-devkitc-1/firmware.bin|elf), if asked for
        ├── diagram.png      ← screenshot of the circuit from the Wokwi editor (the author takes it), with diagram.json
        ├── schematic.svg    ← the real circuit as a schematic (schemdraw, script outside the repo), if asked for
        └── README.md        ← what the step does, wiring, what breaks on real hardware; the homework's README also
                               holds the assignment, translated in full, and how each point is done
```
The homework is **one folder per board** (`NN-homework-<slug>-esp32`, maybe `…-stm32` too): the final code of every
task, required and optional. Module 1 split homework into `base` and `extra` steps; it stays as it is.
Module 0 (self-study) is not in the repo.
Keep every project standard: **never** multiple environments, `src_dir` or `build_src_filter`.

**STM32 steps** (Black Pill STM32F411CE + ST-Link V2, from 1.8 on) are STM32CubeIDE projects, not PlatformIO ones.
The step folder, `<step>.ioc` and the Eclipse project share one name: CubeMX generates into `<location>/<project name>`,
and an Eclipse workspace can't hold two projects with the same name. CubeMX Toolchain/IDE = STM32CubeIDE (its default
is EWARM). Tracked: the `.ioc` (pins, clock: the source of truth), `Core/`, `Drivers/` (the HAL CubeMX copies in, so a
clone builds without CubeMX), the `*.ld` linker scripts, `.project`, `.cproject`, `.mxproject`, `.settings/`,
`README.md`. Ignored: `Debug/`, `Release/`, `*.launch` (absolute paths), `.osx.project`,
`.settings/language.settings.xml` (the indexer's cache: its `env-hash` changes per machine, CubeIDE re-creates it).
`.gitattributes` stores `Core/` and `.mxproject` (CubeMX writes CRLF) with LF.
Own code only between `USER CODE BEGIN/END`: a regenerate rewrites the rest.
CubeIDE opens a step in place (File → Open Projects from File System, no copy into the workspace).

**Two boards (from module 2 on).** Homework is written for the ESP32, and the author may do a step on the Black Pill
too. The board ends every step's name, even when only one version exists, so a second one never forces a rename:
`NN-<type>-<slug>-esp32` (PlatformIO) or `NN-<type>-<slug>-stm32` (STM32CubeIDE). The same task on both boards shares
the number and the slug, and takes one row in the lesson's steps table with a link per board. Each version is a
standalone project: no code shared between the boards (that would need `src_dir`-style setups and linked folders);
the logic is copied. The second version's README links the first one for the task and describes only what differs:
pins, clock, interrupts, logging, what breaks on that board. ESP32 work starts in the lesson's draft; an STM32
version is generated by CubeMX straight into its step folder, because renaming a CubeIDE project means renaming the
folder, the `.ioc` and the Eclipse project together. An STM32 step whose name repeats another lesson's can't be open
in the same CubeIDE workspace. Module 1 keeps its names.

## Tooling

- VS Code: open `EMB27.code-workspace` (multi-root). PlatformIO only sees a project when `platformio.ini`
  sits at the root of a workspace folder, so the workspace holds the repo root plus exactly one project:
  the current lesson's draft or, between lessons, the last step of the lesson just finished (with no project
  the PlatformIO buttons disappear). The next draft replaces that step. Build other saved steps with
  `pio run -d <step-folder>`.
  IntelliSense for saved steps comes from a local root `.vscode/c_cpp_properties.json` (ignored by git):
  `pio project init --ide vscode -d <any-step>`, then copy its `.vscode/c_cpp_properties.json` to the root.
  Regenerate it when a step adds libraries (`lib_deps`) or another board.
- `build_src_flags = -Wall -Wextra` in every project: warnings for our own code only, not for the framework.
- The root `.gitignore` is the only tracked one and covers `.pio/` and `.vscode/` everywhere.
  PlatformIO re-creates a `.gitignore` in each project whenever it regenerates IntelliSense
  (`pio project init --ide vscode`), so the root file ignores `/module-*/**/.gitignore`.
- Wokwi (VS Code extension): each step has `wokwi.toml` (paths relative to the toml) and `diagram.json`
  (the circuit = hardware, not code). ▶ in a step's diagram editor uses the `wokwi.toml` next to it.
  The visual editor only opens for `diagram.json` / `diagram.*.json`.
  Wokwi doesn't compile: change the code → Build → ▶.

## Board

- A DevKitC-1 clone in the style of VCC-GND Studio YD-ESP32-S3: module ESP32-S3-WROOM-1 **N16R8**
  (16 MB flash, 8 MB octal PSRAM, which takes GPIO 35–37), two USB-C ports (native USB and a CH343P USB-UART bridge).
- Onboard LEDs: **not verified yet**, sources disagree. The course (lesson 1.4) says GPIO 48 = blue SYS-LED and
  GPIO 47 = WS2812 RGB; YD-ESP32-S3 pages put the WS2812 on GPIO 48 behind an `RGB` solder jumper. Observed in 1.3:
  the RGB LED stayed dark while GPIO 48 toggled. Wokwi's DevKitC-1 has its RGB LED on GPIO 38.
- PlatformIO's `esp32-s3-devkitc-1` assumes 8 MB flash and no PSRAM: fine until a project needs more.
- Native USB: `/dev/cu.usbmodem*`, VID:PID `303A:xxxx` (Espressif). On macOS the CH343P UART bridge shows up as
  `/dev/cu.usbmodem*` too (VID:PID `1A86:55D3`): tell the ports apart by the VID. `Serial` (UART0) is on the UART port.
- Arduino core 2.0.17: `ledcSetup`/`ledcAttachPin`, not the 3.x API (`ledcAttach`).
  Check the version before trusting any documentation or generated code.
- Pins to avoid: 19/20 (native USB), 0/3/45/46 (strapping), 26–37 (flash/PSRAM), 43/44 (UART0).

## Git

- One repository; lesson work is never committed to `main` directly. Each lesson (or extra project) gets its own
  branch, named after its folder (`module-N/N.M-<topic>`), created from an up-to-date `main` before its first commit:
  the draft, `practice` and `lecture` steps, the lesson README, module docs. Commit to it after every step that works.
- **The homework gets a branch of its own,** `module-N/N.M-<topic>-homework`, so the pull request the teacher reviews
  holds nothing but the homework: its folder and its row in the lesson README. The teacher may see the practice and
  lecture steps on `main`, just not in that pull request. Order: the homework is written in the draft on the lesson
  branch, one commit per change; `/save-hw` saves the practice and lecture steps, removes the draft, and the lesson
  branch is merged; then the homework branch starts from the updated `main` and replays the draft's homework commits
  into the homework folder (same messages and author dates, each one built), plus a README commit. Notes written
  after the homework (`module-N/CLAUDE.md`, `module-N/README.md`) go to `main` as `docs:` commits, never into the
  homework pull request.
  Format: `<folder>: <what was done>` (`repo:` for root configs, `docs:` for documentation).
- Remote `origin` = https://github.com/alex-rns/EMB27, public. Push the branch after committing.
  The history is published: no rewriting and no force-push unless the author explicitly asks.
- A branch reaches `main` only through a pull request (`gh pr create --base main`), and CI runs on it.
  The homework branch goes to the teacher: `/save-hw` opens its PR, the author sends the link to the teacher,
  review fixes are new commits on the homework branch, and the merge waits for the teacher's review and the author's
  yes. Every other branch (the lesson branch, an extra project the author says is not homework) skips the review:
  the author merges it without waiting for CI, and nobody waits for that CI run afterwards either.
  Always a merge commit (`gh pr merge --merge --delete-branch`), never squash: every step's commit stays in the
  history. Repo configs and docs outside a lesson (`repo:`, `docs:`) may still go straight to `main`.
- CI (GitHub Actions, `.github/workflows/build.yml`) runs on every push to `main` and on every pull request:
  `.github/scripts/checks.sh` (no Cyrillic, valid `diagram.json`, no `secrets.h`) and `.github/scripts/build-all.sh`
  (builds every folder with a `platformio.ini`; a compiler warning in our code fails it). PlatformIO 6.2.0 and
  espressif32 7.1.3 are pinned there; change them together with the local install. Both scripts run locally too
  (`PIO=~/.platformio/penv/bin/pio bash .github/scripts/build-all.sh`).
  STM32 steps get `checks.sh` but no build yet: build them in CubeIDE (or its headless build) before committing.
  A step's `wokwi-scenario.yaml` runs locally only (`wokwi-cli . --scenario wokwi-scenario.yaml` in the step folder),
  not in CI: the free Wokwi plan has 50 minutes per rolling 30 days, shared with the Wokwi MCP.
