---
name: new-task
description: Use when the user adds a homework step to an existing EMB27 lesson (an optional extra, the base task or a lecture demo), runs /new-task, or wants a new variant of a task kept alongside the previous one.
---

# New task

## Overview

A lesson's homework is a sequence of task folders `tasks/NN-<type>-<slug>/`. Each one has its own
PlatformIO environment, Wokwi circuit and README. Earlier tasks stay untouched, so every step can
still be built and simulated later.

Talk to the user in Ukrainian. Everything written into the repo is English.
The user invoked the whole procedure, so it counts as one step: do steps 1–5, then report and stop.

## Inputs

Take them from the arguments (`/new-task extra third-led`) or the conversation.
- lesson `L` = `module-N/N.M-<topic>`: the one the user is working in (the open file, the latest lesson
  in `EMB27.code-workspace`, or ask);
- type: `base` (required homework), `extra` (optional complication), `lecture` (demo from the class);
- slug: kebab-case English, what this task adds (`one-gpio`, `third-led`). If it's missing, choose one
  and say in the report that the user can rename it before the commit;
- base task to copy from: the one the user names ("like 05"), otherwise the highest-numbered task.

Below, `<task>` = the new folder name `NN-<type>-<slug>`.

## Steps

1. `NN` = the highest existing task number + 1, two digits. Never renumber existing tasks.
2. Create `L/tasks/<task>/`:
   - `main.cpp`: a copy of the base task's `main.cpp`. Make the change yourself only if the user asked
     you to write it; otherwise leave the copy and the change is theirs (hardware work: the user types,
     and 🔴-list items are always typed by the user);
   - `diagram.json`: a copy of the base task's circuit, updated for any parts or wiring the change needs;
   - `wokwi.toml`: firmware and elf at `../../.pio/build/<task>/firmware.bin|.elf`;
   - `README.md`: `# NN — <Type>: <title>`, what changes relative to the base task, wiring
     (a `| Part | GPIO |` table or "same as [NN](../NN-…/)"), **Notes** on what breaks on real hardware.
3. `L/platformio.ini`: append the environment and make it the default:
   ```ini
   [env:<task>]
   build_src_filter = +<<task>/>
   ```
   and `default_envs = <task>` in `[platformio]`.
4. Docs that count or list tasks:
   - `L/README.md`: add the task's row to the tasks table;
   - `module-N/README.md`: the lesson row's summary (e.g. "base task + K extras");
   - `module-N/CLAUDE.md`: the lesson's status/summary if it counts or lists tasks,
     and "Pins used so far" if the circuit uses new pins.
5. Verify:
   ```bash
   rm -rf L/.pio/build/<task>/src           # force our sources to recompile, so warnings show
   ~/.platformio/penv/bin/pio run -d L -e <task> 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'
   python3 -m json.tool L/tasks/<task>/diagram.json > /dev/null
   # Cyrillic in the repo (tracked + new files) — must print nothing:
   git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
   ```
   Explain every compiler warning to the user (CLAUDE.md rule), then fix it or say why it stays.
6. Report to the user (Ukrainian):
   - the folder created, and that its env is now the default;
   - choices you made yourself (slug, what you changed in the code);
   - each command you ran, explained in a line, with what its output means;
   - how to simulate (Build, then ▶ in the task's `diagram.json`), and what to rewire on the breadboard
     if the circuit changed;
   - a suggested commit message `module-N/N.M-<topic>: add <task>`. Commit and push after the user approves it.

## Common mistakes

| Mistake | Consequence |
|---|---|
| Editing the base task instead of copying it | The step history is lost; the old task no longer builds as it was |
| `default_envs` not switched | Upload flashes the old task |
| `wokwi.toml` copied without changing the env path | ▶ simulates the previous task |
| Circuit changed in code but not in `diagram.json` | Wokwi shows the old wiring |
| Module README/CLAUDE.md task counts not updated | "base + 4 extras" stays wrong after the 5th |
| Renumbering tasks | Breaks README links and environment names |
| `grep -P` for the Cyrillic check | Stock macOS grep has no `-P`; use the perl line above |
