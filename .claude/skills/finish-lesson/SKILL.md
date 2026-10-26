---
name: finish-lesson
description: Use when the user runs /finish-lesson or asks for a lesson's check questions or wrap-up in the EMB27 repo. Optional, after the homework's pull request is open; never a step the homework waits for.
---

# Finish lesson

## Overview

The point of finishing a lesson is that the user **understands the topic**: every saved step still builds, the
docs match the code, and the user has answered check questions in their own words. This skill is **optional** and
runs only when the user asks. The homework's pull request is already open by then (`/save-hw` opens it right after
saving); it comes from the homework branch `module-N/N.M-<topic>-homework` and holds only the homework (CLAUDE.md,
Git), so doc fixes for the homework become new commits on that branch and show up in the open pull request.
If the homework isn't saved yet, run `/save-hw` first.

Steps 1–3 run right away and count as one step: report them in one message and stop.
Step 4 onwards waits for the user's answers.

Talk to the user in their language (see their CLAUDE.md). Everything written into the repo is English.

## Steps

1. **Build every step.** Each step folder is an ordinary PlatformIO project. Remove our compiled sources
   first: an unchanged file isn't recompiled, and its warnings never show.
   ```bash
   L=module-N/N.M-<topic>
   for s in $L/*/; do
     [ -f "$s"platformio.ini ] || continue   # STM32 steps: built separately, below
     rm -rf "$s".pio/build/*/src
     echo "== $s"; ~/.platformio/penv/bin/pio run -d "$s" 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'
   done
   ```
   STM32 steps (`-stm32`, from module 2 on) are CubeIDE projects: build each with the headless build from
   lesson 1.8's README ("Working with the steps", CubeIDE closed) and check its output for `warning:` the same way.
   Every step must succeed. Explain every warning to the user, then fix it or say why it stays.
2. **Consistency checks:**
   - every PlatformIO step has `platformio.ini` (one `[env:esp32-s3-devkitc-1]`, `build_src_flags = -Wall -Wextra`),
     `src/main.cpp`, `README.md`; a step with circuits also has `diagram.json` (valid JSON) and `wokwi.toml`
     (→ `.pio/build/esp32-s3-devkitc-1/…`);
   - pins in each `diagram.json` (where a step has one) match the pins in its `main.cpp`. Don't open the `diagram.png` screenshots:
     viewing them is slow, and `/save-hw` already checked each one when it was taken;
   - each `schematic.svg` (where a step has one) shows the parts and GPIOs of its README and `main.cpp`: read
     its `<text>` labels, don't render it;
   - every claim in the step READMEs (timings, ranges, pins) matches the code;
   - the lesson README steps table lists every step;
   - no Cyrillic in the repo (tracked + new files); this must print nothing:
     ```bash
     git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'BEGIN { @ARGV = grep { -T } @ARGV } print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
     ```
   - `git status --short` shows no `.pio/`, `.vscode/` or project `.gitignore`.

   A mismatch between code and README is a finding. Don't fix it silently. If the code is wrong, make it
   a practical task for the user (spotting mistakes in generated code is their role). If only the doc
   is wrong, list it for step 5.
3. **Check questions** (CLAUDE.md rule): 3–5 questions to answer in their own words, plus
   1–2 practical tasks. Take them from what this lesson's steps actually did. From the "what to catch"
   list in `module-N/CLAUDE.md`, use only the items this lesson touched. In the same message, show each
   command from steps 1–2 with a one-line explanation of what its output meant.
   Then **stop and wait for the answers**. A lesson already marked ✅ without recorded answers still
   counts as unfinished.
4. **Review the answers.** Confirm what is right and correct misconceptions with an analogy.
   Record anything still shaky as an open thread.
5. **Update the docs** (English). The homework's pull request holds only the homework, so the docs go to
   three places:
   - **the homework branch** `module-N/N.M-<topic>-homework`: the homework folder's README and its row in the lesson
     `README.md`: fix the doc findings from step 2; add Wokwi links if the user published them. These commits show
     up in the open pull request;
   - **`main`, one `docs:` commit** (`git switch main && git pull`; the pre-commit hook allows files outside lesson
     folders), never in the homework's pull request:
     - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — homework`, plus ` + P practice` /
       ` + L lecture` when the lesson has those steps;
     - `module-N/CLAUDE.md`: status ✅ and a technical lesson summary: concepts and what breaks on real
       hardware. Update open threads. The repo is public: how the user answered, their mistakes and their
       learning progress never go into repo files;
     - root `README.md`: set the module status to `done` after its last lesson;
     - root `CLAUDE.md`: update "Next step" if it changed. Lesson summaries never go into the root file;
   - **a new lesson branch** `module-N/N.M-<topic>` from `main`, only when a `practice` or `lecture` step needs a fix,
     or the lesson's draft is still there (move it to the Trash and put the lesson's last step in its place in
     `EMB27.code-workspace`, so one PlatformIO project stays open). Its pull request has no review: merge it after
     the user's yes without waiting for CI.
6. **Commit and push.** Suggest the messages; the user approves or rewrites them (CLAUDE.md):
   `module-N/N.M-<topic>: finish lesson` on the homework branch (only when it has changes) and
   `docs: module-N: finish lesson N.M` on `main`. Push both. `git status -sb` on the homework branch must show
   `## module-N/N.M-<topic>-homework...origin/module-N/N.M-<topic>-homework` with nothing ahead.
7. **The homework's pull request** is already open. After pushing to the homework branch, check that the pull
   request still holds nothing but the homework: `gh pr view <n> --json files --jq '.files[].path'` lists only the
   homework folder and the lesson `README.md`. Anything else (the draft, `archive/`, practice or lecture steps,
   module docs): fix the branch at once. Review fixes are new commits on the homework branch. Merge only after the
   teacher's review and the user's yes. A lesson without homework has only its lesson branch: no review, merge after
   the user's yes.

   Merge with `gh pr merge --merge --delete-branch`, then `git switch main && git pull`.

## Common mistakes

| Mistake | Consequence |
|---|---|
| Building without removing `.pio/build/*/src` | Cached objects hide the warnings |
| `grep -P` for the Cyrillic check | Stock macOS grep has no `-P`: it errors and searches nothing |
| Trusting an existing ✅ | A lesson marked done earlier may never have had its questions answered |
| Fixing a code bug yourself during the wrap-up | The user loses the exercise of finding it |
| Marking the lesson done before the answers | Breaks the learning rule in CLAUDE.md |
| Lesson summary written into the root CLAUDE.md | The root file bloats; summaries belong in `module-N/CLAUDE.md` |
| Questions about things the lesson didn't cover | Questions must come from this lesson's work |
| Lesson work committed to `main` directly | It skips CI on the branch and, for homework, the teacher's review |
| Homework merged before the teacher's review | The teacher reviews code that is already in `main` |
| Squash merge | The per-step commits the teacher can walk through collapse into one |
| Practice, lecture steps or module notes in the homework's pull request | The teacher reviews things that aren't the homework (the author's rule) |
| Homework branch started before the lesson branch is merged | The lesson branch's commits (draft, practice, lecture) land in the teacher's pull request |
