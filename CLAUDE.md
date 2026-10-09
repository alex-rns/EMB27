# EMB27 — project rules

> Rules for AI assistants working in this repository: homework for the Beetroot Academy
> "Embedded Development" course, group EMB27. Module-specific technical notes live in `module-N/CLAUDE.md`.

## Language

Everything in the repository is in English: code, comments, docs, configs, commit messages.
Personal notes about the author never go into this repository; it is public.

## Workflow

- Experiments happen in an ordinary PlatformIO project created by the wizard: one `src/main.cpp`,
  edits, copies, "archive" folders. No rules while experimenting.
- `/save-hw` + the assignment turns a finished homework into the saved structure below: steps from the
  assignment, code for each step, Wokwi circuits, READMEs. The experiment project goes to the macOS Trash.
  Commit and push after the author confirms.
- `/finish-lesson` builds every step, checks code against circuits and READMEs, asks check questions,
  then updates the docs and commits.
- Project skills live in `.claude/skills/` (tracked). Follow a skill when it is invoked or the situation matches it.

## Repository layout

The layout mirrors the course: module (0–7) → lesson (N.M) → homework steps.
```
README.md                    ← course, board, table of modules (GitHub front page)
CLAUDE.md                    ← this file
module-N/
├── README.md                ← module topics + table of lessons with links
├── CLAUDE.md                ← technical notes for this module: lessons → folders → status,
│                              hardware/pins, what to watch for, known issues
└── N.M-<topic-slug>/        ← one folder per lesson with homework (a container, not a project)
    ├── README.md            ← assignment + table of steps (type, what it shows)
    └── NN-<type>-<slug>/    ← one step = one ordinary PlatformIO project; type = lecture | base | extra
        ├── platformio.ini   ← the wizard's 4 lines + build_src_flags = -Wall -Wextra
        ├── src/main.cpp
        ├── diagram.json, wokwi.toml (→ .pio/build/esp32-s3-devkitc-1/firmware.bin|elf)
        └── README.md        ← what the step does, wiring, what breaks on real hardware;
                               "▶ Simulate in Wokwi" link once published (no images)
```
Module 0 (self-study) is not in the repo.
Keep every project standard: **never** multiple environments, `src_dir` or `build_src_filter`.

## Tooling

- VS Code: open `EMB27.code-workspace` (multi-root). PlatformIO only sees a project when `platformio.ini`
  sits at the root of a workspace folder, so the workspace holds the repo root plus the project being
  experimented in. Saved steps are not in the workspace: build them with `pio run -d <step-folder>`.
- `build_src_flags = -Wall -Wextra` in every project: warnings for our own code only, not for the framework.
- The root `.gitignore` is the only tracked one and covers `.pio/` and `.vscode/` everywhere.
  PlatformIO re-creates a `.gitignore` in each project whenever it regenerates IntelliSense
  (`pio project init --ide vscode`), so the root file ignores `/module-*/**/.gitignore`.
- Wokwi (VS Code extension): each step has `wokwi.toml` (paths relative to the toml) and `diagram.json`
  (the circuit = hardware, not code). ▶ in a step's diagram editor uses the `wokwi.toml` next to it.
  The visual editor only opens for `diagram.json` / `diagram.*.json`.
  Wokwi doesn't compile: change the code → Build → ▶.

## Board

- ESP32-S3-DevKitC-1, looks like revision v1.1: onboard RGB on GPIO 38 (also in Wokwi), GPIO 48 is free.
  Native USB: `/dev/cu.usbmodem*`, VID:PID `303A:xxxx` (Espressif).
- Arduino core 2.0.17: `ledcSetup`/`ledcAttachPin`, not the 3.x API (`ledcAttach`).
  Check the version before trusting any documentation or generated code.
- Pins to avoid: 19/20 (native USB), 0/3/45/46 (strapping), 26–37 (flash/PSRAM), 43/44 (UART0).

## Git

- One repository; everything on `main`. Commit after every step that works.
  Format: `<folder>: <what was done>` (`repo:` for root configs, `docs:` for documentation).
- Remote `origin` = https://github.com/alex-rns/EMB27, public. Push after committing.
  The history is published: no rewriting and no force-push unless the author explicitly asks.
- CI is planned: the course final project requires GitHub + CI/CD. Build every step project
  (every folder with a `platformio.ini`).
