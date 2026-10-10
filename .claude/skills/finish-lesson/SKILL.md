---
name: finish-lesson
description: Use when the user says a lesson or its homework is done in the EMB27 repo, runs /finish-lesson, or is about to move on to the next lesson.
---

# Finish lesson

## Overview

The point of finishing a lesson is that the user **understands the topic**. A lesson is finished when
every saved step still builds, the docs match the code, the user has answered check questions in their
own words, and the result is committed and pushed. If the homework isn't saved yet, run `/save-hw` first.

Steps 1–3 run right away and count as one step: report them in one message and stop.
Step 4 onwards waits for the user's answers.

Talk to the user in their language (see their CLAUDE.md). Everything written into the repo is English.

## Steps

1. **Build every step.** Each step folder is an ordinary PlatformIO project. Remove our compiled sources
   first: an unchanged file isn't recompiled, and its warnings never show.
   ```bash
   L=module-N/N.M-<topic>
   for s in $L/*/; do
     rm -rf "$s".pio/build/*/src
     echo "== $s"; ~/.platformio/penv/bin/pio run -d "$s" 2>&1 | grep -E 'warning:|error:|SUCCESS|FAILED'
   done
   ```
   Every step must succeed. Explain every warning to the user, then fix it or say why it stays.
2. **Consistency checks:**
   - every step has `platformio.ini` (one `[env:esp32-s3-devkitc-1]`, `build_src_flags = -Wall -Wextra`),
     `src/main.cpp`, `diagram.json` (valid JSON), `wokwi.toml` (→ `.pio/build/esp32-s3-devkitc-1/…`), `README.md`;
   - pins in each `diagram.json` match the pins in its `main.cpp`. Don't open the `diagram.png` screenshots:
     viewing them is slow, and `/save-hw` already checked each one when it was taken;
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
5. **Update the docs** (English):
   - lesson `README.md`: fix the doc findings from step 2; add Wokwi links if the user published them;
   - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — base task + K extras`,
     plus ` + P practice` when the lesson has practice steps;
   - `module-N/CLAUDE.md`: status ✅ and a technical lesson summary: concepts and what breaks on real
     hardware. Update open threads. The repo is public: how the user answered, their mistakes and their
     learning progress never go into repo files;
   - root `README.md`: set the module status to `done` after its last lesson;
   - root `CLAUDE.md`: update "Next step" if it changed. Lesson summaries never go into the root file.
6. **Commit and push.** Suggest `module-N/N.M-<topic>: finish lesson`; the user approves or rewrites it
   (CLAUDE.md). Then `git push`. `git status -sb` must show `## main...origin/main` with nothing ahead.

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
