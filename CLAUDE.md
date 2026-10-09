# Context: moving from web development to Embedded

> This file carries context between sessions. Claude Code reads it automatically
> when working in `~/embedded/EMB27`. Update it after every completed lesson.
> Last updated: 2026-10-10

## Who I am

Ruby on Rails + TypeScript developer with commercial experience, moving into Embedded.

**I know:** Git, basic Linux CLI, testing, CI, APIs/protocols (HTTP, JSON),
breaking down tasks, reading documentation.

**I don't know yet:** C at a confident level, manual memory management, pointers,
electronics, circuit design, hands-on work with microcontrollers.
Never worked without a garbage collector or without an OS.

**Goal:** junior / strong junior Embedded Developer position, Ukraine or remote.
Priority: defence tech (UAV, ground stations, communications), then IoT
and industrial automation.

**Why the switch:** the web market has shrunk (layoffs, fewer openings),
defence tech and military tech are growing, and Ukraine really needs this right now.

## How to work with me — IMPORTANT

- **Talk to me in Ukrainian.**
- **Everything in the repository is in English** — code, comments, docs, configs,
  commit messages (my decision, 2026-10-10). Only the conversation is in Ukrainian.
- Explain in detail, "like to a child": analogies, what physically happens
  in memory/the CPU, why it matters specifically in embedded.
- **One step at a time.** No week-long plans. After each step,
  stop and wait for my confirmation.
- **Don't dive into instruction encoding and hexdumps for their own sake.**
  Lesson 1 overdid this — interviews don't ask about it.
  What matters: concepts + what exactly breaks on real hardware.
- Terminal commands: **first write the command and explain what it does**,
  then run it, then explain every line of the output.
- Compile with `-Wall -Wextra -g`. Explain every warning.
- End every lesson with 3–5 check questions to answer "in my own words"
  + 1–2 practical tasks. Don't move on without my answers.
- Draw parallels with Ruby/TypeScript and **say explicitly where web thinking hurts**.
- Commits: I write the messages, you suggest the format.

### On writing code — our agreement

I said AI would write the code. We agreed:
- the items from the 🔴 list below (2–5 lines each) **I type myself**,
  because embedded technical interviews make you write C without AI,
  and that is the main filter;
- demos and large code are written by the assistant;
- my main role is to **spot the mistake in generated code**.

Don't revisit this agreement unless I explicitly ask.

## Environment

- Mac, Apple Silicon (arm64). `gcc` = **Apple clang**, not GNU GCC.
- Command Line Tools installed. `objdump`/`nm` are the LLVM versions, `otool` is available.
- `code` CLI is **not installed**.
- **Working folder and repository: `~/embedded/EMB27`** (EMB27 = the course group). Course only.
- My own C track (`~/embedded/c-lessons/lessonNN`) and `~/embedded/masterclass-c++-Sep16`
  live **outside the repository and outside git**. My decision, 2026-10-10.

**Git — agreement (2026-10-10):**
- **one repository** `~/embedded/EMB27` (it will be part of my portfolio). No separate git per module.
  History started fresh on 2026-10-10; the old one, with c-lessons, was deleted;
- each course assignment = its own PlatformIO project folder at the repo root: `module-1.3`, `module-1.4`, …
  In VS Code, open **`EMB27.code-workspace`** (multi-root: repo + modules).
  PlatformIO only sees a project when `platformio.ini` sits at the root of a workspace folder,
  so a new module = a new entry in this file's `folders`;
- every module's `platformio.ini` has `build_src_flags = -Wall -Wextra`
  (warnings for our own code only, not for the Arduino framework);
- **module layout (agreed 2026-10-10)** — a homework = base task + optional extras, so:
  ```
  module-X.Y/
  ├── README.md            ← assignment + table of tasks (type, what it shows)
  ├── platformio.ini       ← src_dir = tasks; one [env:<task>] per task with
  │                          build_src_filter = +<<task>/>; shared settings in [env]
  └── tasks/NN-<type>-<slug>/   type = lecture | base | extra
      ├── main.cpp, diagram.json, wokwi.toml (→ ../../.pio/build/<task>/firmware.bin)
      ├── README.md        ← what was done, wiring, what breaks on real hardware
      └── wokwi.png (+ optional demo.gif) ← screenshot / recording of the running Wokwi
                           simulation (not phone photos); resized, GIF made with ffmpeg
  ```
  New module: delete the `.gitignore` PlatformIO generates; the root `.gitignore` is the only one
  and covers `.pio/` and `.vscode/` everywhere.
  `default_envs` = the task being worked on. Without it, Upload flashes **every** task in turn.
  With it, a plain `pio run` builds only that task, so to build everything list every `-e <env>` (CI too);
- commit after every step that works. Format: `<folder>: <what was done>`
  (`repo:` for root configs, `docs:` for CLAUDE.md/PLAN.md). No branches needed, everything on `main`;
- remote `origin` = https://github.com/alex-rns/EMB27 (**public**, since 2026-10-10).
  Push after committing. History is published now: no rewriting, no force-push.

**Wokwi (simulator in VS Code):** Community License activated. Each task folder has
`wokwi.toml` (paths are relative to the toml itself) and `diagram.json` (the circuit = hardware, not code).
▶ in a task's diagram editor uses the `wokwi.toml` next to it; otherwise use `Wokwi: Select Config File`.
The visual editor only opens for `diagram.json` / `diagram.*.json`.
Wokwi doesn't compile: change the code → **Build** → ▶. In Wokwi, the DevKitC-1 onboard RGB is on GPIO 38.

**Hardware:** I have an **ESP32-S3-DevKitC-1** (since 2026-10-08). Looks like revision v1.1: onboard RGB
on GPIO 38, GPIO 48 is free (checked on the board). Connected via native USB:
`/dev/cu.usbmodem*`, VID:PID `303A:xxxx` (Espressif). `pio` is not on the PATH of a regular
terminal: use the PlatformIO terminal or `~/.platformio/penv/bin/pio`.
Arduino core 2.0.17 (`ledcSetup`/`ledcAttachPin`, not the 3.x API).
Earlier: ordered an ESP32 starter kit.
Planned: STM32 Nucleo-F411RE or G431RB, 8-channel logic analyzer
(Saleae clone, PulseView software), BME280 over SPI, multimeter, soldering iron.

## Course

**Beetroot Academy "Embedded Development"**, started 2026-09-16, 5 months,
8 modules, on **ESP32 + C++ via PlatformIO/VS Code**. Lesson recordings are available.

- Module 0 (3 weeks) — self-prep: electronics, components, tools,
  PlatformIO, GitHub, basic C++ syntax
- Module 1 — measurements, blink, GPIO, datasheet, ADC, soldering
- Module 2 — C++ for MCUs, superloop, interrupts, timers, debounce, volatile
- Module 3 — ADC calibration, filtering, DAC, PWM, servo, PWM via registers, encoder
- Module 4 — UART, I2C, SPI, BME280 over SPI, DMA, JTAG/SWD ← the most valuable for the job
- Module 5 — KiCad, routing, Gerber, RC filters, PID
- Module 6 — state machine, FreeRTOS (tasks, queues, mutex), sleep, heap
- Module 7 — final project, **optional port to STM32**

### Flexible slots — the main lever

Modules 2–6 have 5 "deep dive into a topic of your choice" slots. Plan:

| Module | Spend it on |
|---|---|
| 2 | GPIO **via registers** instead of `digitalWrite` |
| 3 | deeper PWM via registers |
| 4 | **GDB/OpenOCD + logic analyzer** ← the most valuable |
| 5 | filtering (no deep KiCad) |
| 6 | task stack sizing, race conditions |

**Ask about the STM32 port (module 7) at the start, not at the end.**

### What the course does NOT cover — we close these gaps in parallel

- plain C (the course goes through the C++/Arduino wrapper)
- STM32 bare-metal without CubeMX
- linker script / startup code / memory map
- Makefile/CMake (PlatformIO hides all of it)
- firmware unit tests (Unity/Ceedling) — also my advantage from web
- GDB/OpenOCD hands-on

## Learning plan

**The detailed breakdown is in `PLAN.md`** (what each item covers,
interview questions, what to catch in AI-generated code, portfolio).

### 🔴 Must know by heart — asked in interviews

| # | Topic | Status |
|---|---|---|
| 1 | `volatile` | ✅ done |
| 2 | bits in a register | ✅ done |
| 3 | `uint8_t`/`uint32_t` — why not `int` | ⏭ next |
| 4 | pointer to a register: `*(volatile uint32_t*)0x40020014` | ⏭ next (together with 3) |
| 5 | `static` / `const` / `extern` | — |
| 6 | stack vs heap, why embedded avoids `malloc` | — |
| 7 | ISR: what you must not do in an interrupt | — |
| 8 | `.text` / `.data` / `.bss` — what lives in flash, what in RAM | — |
| 9 | `struct` and padding | — |
| 10 | signed/unsigned traps | — |

### 🟡 Read and catch mistakes (don't write from scratch)

Peripherals: GPIO (pull-up, push-pull/open-drain), interrupts, timers/PWM,
ADC, UART (baud, `\r\n`), I2C (**mandatory pull-up resistors — AI won't add them
because it can't see the board**), SPI (modes 0–3), DMA, reading datasheets.

Tools: logic analyzer (the main way to verify AI code),
GDB/OpenOCD, multimeter, Makefile/linker script — read and modify.

### 🟢 AI generates, I review

Init boilerplate, parsers, buffers, FreeRTOS skeletons, Makefiles, tests.

## Done so far

**lesson01 — the four stages of compilation.**
`-E` preprocessor (dumb text substitution, 40 files from a single `#include`,
`#define` leaves nothing behind at runtime) → `-S` compiler (C → assembly,
literal merging, the as-if rule, `printf`→`puts` at `-O2`) →
`-c` assembler (mnemonics → bytes, holes instead of external addresses, relocations,
`T`/`U` symbols) → linking (sections → addresses, patching the holes).
Takeaway: on Mac, dynamic linking via `dyld`; on STM32 — **static only**,
because there is no OS to fill in the address. Hence `undefined reference` and the flash cost of `printf`.

**lesson02 — `volatile`.** CPU registers as a "notepad", why the compiler
caches values. Three cases: hardware register, interrupt handler, DMA.
The as-if rule and why `volatile` is part of the definition of observable behavior.
Demo at `-O2`: 1 read vs 3; the compiler **removed** the `while(flag==0){}` loop **entirely**.
`volatile` ≠ atomicity: `counter++` is read-modify-write, race condition.
`volatile` controls the compiler, **not** the hardware cache (matters for DMA on ESP32/STM32H7).
Telltale symptom: "added a printf — it works, removed it — it breaks".

**lesson03 — bits in a register.** A register = 32 switches. Hex because 1 digit = 4 bits.
`1u << n` (counting from zero, `1u` because `1<<31` on a signed int is UB).
`|=` set, `&= ~` clear, `^=` toggle, `&` test.
The test yields **2ⁿ, not 1** — that's why `== 1` doesn't work.
The `=` instead of `|=` trap (symptom: "added a second pin — the first one broke").
Multi-bit field: first `&= ~mask`, then `|= value`. Mask = `(1u << n) - 1`.
Diagnosis: wrote to a register, read back zero → **forgot to enable the clock**.

**masterclass-c++-Sep16** (`~/embedded`, outside the repo) — C++ masterclass (classes,
operator overloading, the `Money` class), the ETL library.

**module-1.3 (course, 2026-10-09) — first hardware + "police light".**
Board connected, port and VID:PID explained, serial monitor (ESP-IDF factory demo).
The preprocessor cuts code under `#if`, so all flags at `0u` give an empty `loop()`.
`pinMode` after `ledcAttachPin` detaches PWM from the pin. Dangerous S3 pins:
19/20 (USB), 0/3/45/46 (strapping), 26–37 (flash/PSRAM), 43/44 (UART0).
220 Ω resistor: blue and white are dimmer (Vf ≈ 3 V at 3.3 V). Break-before-make.
Police light: 3 LEDs, double flashes, variable speed (`#define` → variable, global
because `loop()` runs again and again; `int`, not `uint32_t`, because of underflow → a 49-day `delay`).
**Two LEDs on one pin:** 3V3→LED→pin (active-low) + pin→LED→GND. Source/sink, push-pull.
Key idea I **formulated myself**: current flows only where there is a voltage *difference*;
the pin doesn't "decide" anything, it just becomes either plus or minus. Current flows through a diode only from + to −.
I know that two outputs, one HIGH and one LOW, wired together make a short circuit (hence open-drain in I2C).
Tasks: `module-1.3/tasks/00…05` — lecture demo, base, 4 extras; each with code, circuit and README.
Tasks 01–03 were reconstructed from the session (they were only intermediate edits of main.cpp).

## Open threads

1. **I haven't answered the 5 questions from lesson03** (bits) — worth coming back to:
   set/clear bit 7, what number `reg & (1u<<7)` gives, why a 2-bit
   field takes two lines, what to check if a register reads back as zero.
2. module-1.3: refactoring the police light (pattern as a bit table, e.g. `0b011`) was proposed
   but not done. A good bridge to lesson03 and to the next lesson on `uint8_t`.

## Next step

Lesson: **how to reach a register at an address from C** — break down the line
`#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)` symbol by symbol,
plus `uint8_t`/`uint32_t` and why not `int`. These are items 3 and 4 of the 🔴 list.
