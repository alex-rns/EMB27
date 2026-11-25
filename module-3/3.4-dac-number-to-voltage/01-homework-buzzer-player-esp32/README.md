# 01 — Homework: a non-blocking buzzer player on a fixed tick, and simple melodies (ESP32-S3)

[← Lesson 3.4](../)

The lesson 3.4 homework, both tasks: a passive buzzer plays melodies through LEDC PWM, driven by a fixed 50 ms tick
from an `esp_timer`, with no FreeRTOS task of our own and no delay; the five melodies of the assignment play one
after another while a second timer blinks an LED, to show that nothing waits. ESP-IDF in C (PlatformIO, ESP-IDF 6.1);
the git history of this folder builds it one change per commit.

```
esp_timer, every 50 ms ──► on_tick(): a melody over? start the next ──► player_tick(): count the note's ticks,
                                                                        start the next note when they run out
                                                                                 │
                                                buzzer_tone(f) / buzzer_off() ──► LEDC: f Hz, 50 % ──► GPIO 15 ──► buzzer
esp_timer, every 250 ms ──► on_blink(): toggle GPIO 6 ──► LED
```

## Assignment

Homework: playing sound on a buzzer (PWM).

The code and the result go to GitHub. The repository name (or the branch, or a commit message) must contain the module
number and the lesson number. The link to the repository goes into the LMS.

Choose one task; doing both is not required (both are done here).

**Task 1.** Write a non-blocking audio player for a buzzer that:

- works through PWM (LEDC);
- uses a fixed time tick, for example 50 ms;
- doesn't block the program;
- uses no FreeRTOS task and no delay.

**Task 2.** Write a synthesizer of 2, 3 or more notes, able to play a simple melody. Example melodies:

- Baby Shark: C D E | C D E | C D E
- Jingle Bells: E E E | E E E | E G C D E
- We Will Rock You: C C D | C C D | C C D E
- Twinkle Twinkle Little Star: C C G G | A A G, or simplified C C D D | E E D
- Old MacDonald: G G G D E E D, or simplified C C C D E E D

## How each point is done

| Point | Where | How |
|---|---|---|
| Task 1: through PWM (LEDC) | `buzzer.c` | LEDC timer 0 / channel 0 on GPIO 15, 10 bits. A note is the timer's frequency (`ledc_set_freq()`) at 50 % duty (512 of 1024), the loudest square wave for a passive buzzer; silence is duty 0 |
| Task 1: a fixed tick, 50 ms | `main.c` | `esp_timer_start_periodic()` calls `on_tick()` every 50 ms; every note length is a whole number of ticks |
| Task 1: doesn't block | `player.c`, `main.c` | `player_start()` only stores the melody and returns; `player_tick()` does one tick's work and returns. `app_main()` starts the timers and returns. A second `esp_timer` blinks an LED on GPIO 6 every 250 ms while the music plays, on its own rhythm |
| Task 1: no FreeRTOS task, no delay | the whole project | no `xTaskCreate`, no `vTaskDelay`, no busy loop: the timers' callbacks do all the work. They run in ESP-IDF's own `esp_timer` service, so they stay short |
| Task 2: a synthesizer of several notes | `notes.h`, `player.c` | eight notes C4..C5 (262..523 Hz, the lesson's table) and a rest; a melody is an array of `{frequency, ticks}` |
| Task 2: play simple melodies | `melodies.c`, `main.c` | all five example melodies in eighths (4 ticks), quarters (8) and halves (16), each ending with a rest; when one is over the tick starts the next and logs its name |

The last tick of every note is silent (a quarter is 350 ms of tone and 50 ms of silence), so repeated notes such as
Twinkle's C C or Jingle Bells' E E E are heard as separate notes.

## Wiring

| Part | Connection |
|---|---|
| Passive buzzer module, `S` | 220 Ω → GPIO 15 |
| Passive buzzer module, `-` | `GND` |
| Passive buzzer module, middle pin | not connected |
| LED | GPIO 6 → 220 Ω → LED → `GND` |
| USB | the board's UART port (the CH343P bridge), 115 200 baud |

![Schematic](schematic.svg)

## Checked on the board

The serial log after flashing:

```
I (280) player: started; app_main returns, the timers keep the music and the LED going
I (330) player: Baby Shark
I (5180) player: Jingle Bells
I (11630) player: We Will Rock You
```

Baby Shark is 96 ticks (4.8 s) and Jingle Bells 128 ticks (6.4 s); the log shows 4.85 s and 6.45 s between the
names. By ear and eye: the melodies play one after another, repeated notes sound separate, and the LED blinks evenly
4 times a second all the while. The note frequencies were not measured (no logic analyzer at hand).

## Notes

- **Why a timer and not a loop.** A `while (1)` in `app_main()` that checks the time without a delay never lets
  FreeRTOS's idle task run, and the task watchdog then complains every few seconds. A periodic `esp_timer` needs no
  loop: `app_main()` returns and the callbacks run on time.
- **The callbacks must stay short.** Both run in ESP-IDF's `esp_timer` service one after another; a slow one would
  delay the other. Here each only changes a few variables and LEDC registers.
- **A passive buzzer needs the frequency from us.** It is a small speaker: it sounds at the PWM's frequency. An
  active buzzer has its own oscillator and would only beep at its own pitch.
- **Note lengths are multiples of the tick.** At 50 ms an eighth is 200 ms, a quarter 400 ms; a finer rhythm needs a
  shorter tick.
- **The frequencies are rounded to whole hertz** (A4 = 440 Hz, C4 = 262 Hz instead of 261.6 Hz); by ear it makes no
  difference.

## Working with the step

- **Build / Upload:** `pio run -d 01-homework-buzzer-player-esp32` (add `-t upload`), or the PlatformIO buttons with
  this folder in the VS Code workspace.
- **Serial Monitor:** the UART USB-C port, 115 200 baud (set in `platformio.ini`).
