---
name: finish-lesson
description: Use when the user says a lesson or its homework is done in the EMB27 repo, runs /finish-lesson, or is about to move on to the next lesson.
---

# Finish lesson

## Overview

A lesson is finished when every task still builds, the docs describe what was done, the user has
answered check questions in their own words, and the result is committed and pushed.
Steps 1–3 run right away and count as one step: report them in one message and stop.
Step 4 onwards waits for the user's answers.

Talk to the user in Ukrainian. Everything written into the repo is English.

## Steps

1. **Build every task.** A plain `pio run` builds only `default_envs`, so list every environment.
   Remove our compiled sources first: an unchanged file isn't recompiled, and its warnings never show.
   ```bash
   L=module-N/N.M-<topic>
   rm -rf $L/.pio/build/*/src
   ~/.platformio/penv/bin/pio run -d $L $(grep -o '^\[env:[^]]*' $L/platformio.ini | sed 's/\[env:/-e /') 2>&1 \
     | grep -E 'warning:|error:|SUCCESS|FAILED'
   ```
   Every environment must succeed. Explain every warning to the user, then fix it or say why it stays.
2. **Consistency checks:**
   - every `tasks/*/` has `main.cpp`, `diagram.json` (valid JSON), `wokwi.toml` pointing at its own env, `README.md`;
   - every task folder has `[env:<folder>]` with `build_src_filter = +<<folder>/>`, and every env has a folder;
   - pins in each `diagram.json` match the pins in its `main.cpp`;
   - every claim in the task READMEs (timings, ranges, pins) matches the code;
   - the lesson README tasks table lists every task;
   - no Cyrillic in the repo (tracked + new files); this must print nothing:
     ```bash
     git ls-files -co --exclude-standard -z | xargs -0 /usr/bin/perl -CSD -ne 'print "$ARGV:$.: $_" if /\p{Cyrillic}/; close ARGV if eof'
     ```
   - `git status --short` shows no lesson `.gitignore`, `.pio/` or `.vscode/`.
   A mismatch between code and README is a finding. Don't fix it silently: if it is in code, turn it into
   a practical task for the user (spotting mistakes in generated code is their role); if only the doc is
   wrong, list it for step 5.
3. **Check questions** (CLAUDE.md rule), in Ukrainian: 3–5 questions to answer in their own words, plus
   1–2 practical tasks. Take them from what this lesson's tasks actually did. From the "what to catch" list
   in `module-N/CLAUDE.md`, use only the items this lesson touched. In the same message, show each command
   from steps 1–2 with a one-line explanation of what its output meant (CLAUDE.md style).
   Then **stop and wait for the answers**.
   A lesson already marked ✅ in `module-N/CLAUDE.md` without recorded answers still counts as unfinished.
4. **Review the answers:** confirm what is right, correct misconceptions with an analogy,
   and record anything still shaky as an open thread.
5. **Update the docs** (English):
   - lesson `README.md`: the tasks table is complete; fix the doc findings from step 2;
     add Wokwi links if the user published them;
   - `module-N/README.md`: the lesson row → `[<short name>](N.M-<topic>/) — base task + K extras`;
   - `module-N/CLAUDE.md`: status ✅; a lesson summary (concepts, ideas the user formulated himself,
     mistakes made, what breaks on real hardware, how the check questions went); open threads updated;
   - root `README.md`: module status `done` after the module's last lesson;
   - root `CLAUDE.md`: "Next step", if it changed. Lesson summaries never go into the root file.
6. **Commit and push.** Suggest `module-N/N.M-<topic>: finish lesson`; the user approves or rewrites it
   (CLAUDE.md). Then `git push`. `git status -sb` must show `## main...origin/main` with nothing ahead.

## Common mistakes

| Mistake | Consequence |
|---|---|
| `pio run` without the `-e` list | Only `default_envs` is built; a broken older task goes unnoticed |
| Building without removing `.pio/build/*/src` | Cached objects hide the warnings |
| `grep -P` for the Cyrillic check | Stock macOS grep has no `-P`: it errors and searches nothing |
| Trusting an existing ✅ | A lesson marked done earlier may never have had its questions answered |
| Fixing a code bug yourself during the wrap-up | The user loses the exercise of finding it |
| Marking the lesson done before the answers | Breaks the learning rule in CLAUDE.md |
| Lesson summary written into the root CLAUDE.md | The root file bloats; summaries belong in `module-N/CLAUDE.md` |
| Questions about things the lesson didn't cover | Questions must come from this lesson's work |
