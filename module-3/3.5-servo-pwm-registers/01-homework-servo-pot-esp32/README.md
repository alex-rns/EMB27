# 01 — Homework: a potentiometer sets an SG90 servo 1:1, the angle logged (ESP32-S3)

[← Lesson 3.5](../)

The lesson 3.5 homework: a potentiometer on ADC1 sets the angle of an SG90 servo driven by LEDC PWM, one degree of
the knob for one degree of the shaft, both counted from their leftmost position. The knob turns about 300°, the
servo 180°, so only the first 180° of the knob move the servo; past that the servo stays at 180°. The angle goes to
the console whenever it changes. Like the lecture's code, it is one loop with a 20 ms delay. ESP-IDF in C (PlatformIO,
ESP-IDF 6.1); the git history of this folder builds it one change per commit.

```
app_main(), every 20 ms:
  pot_read_deg() ──► min(angle, 180) ──► servo_set_angle() ──► log the angle if it changed
       │                                        │
  GPIO 1 ──► ADC1: 128 readings averaged,        500 + angle × 2000 / 180 µs ──► LEDC: 50 Hz, 14 bits
             calibrated mV × 300 / 3300,                                         ──► GPIO 18 ──► servo signal
             a 1° backlash against the noise
```

## Assignment

Homework: controlling a servo with PWM.

The code and the result go to GitHub. The repository name (or the branch, or a commit message) must contain the module
number and the lesson number.

Task:

- The potentiometer sets the position of the servo's shaft 1:1.
- Where the servo's and the potentiometer's angles of rotation differ, the difference is cut off, and only the part
  of the range they share is used.
- The angle of deflection from the leftmost position is logged to the console.

## How each point is done

`main.c` marks each point with a `Point 1/2/3` comment.

| Point | Where | How |
|---|---|---|
| The potentiometer sets the shaft 1:1 | `pot.c`, `servo.c`, `main.c` | the knob's angle from its 0 V end is `mV × 300 / 3300` (a linear pot with about 300° of travel, measured below); the servo's angle 0..180° becomes a 500..2500 µs pulse. Both are in degrees from the leftmost position, so one degree of the knob moves the shaft one degree |
| The difference is cut off, the shared range used | `main.c`, `servo.c` | the shared range is 0..180°: `servo_deg = min(pot_deg, 180)`. Past a half turn only the knob moves on. `servo_set_angle()` also holds anything past 180° at 180°, so no caller can push the servo past its end |
| The angle from the leftmost position is logged | `main.c` | `ESP_LOGI` prints `angle N deg from the leftmost position` in whole degrees, only when the number changes, so a still knob keeps the console quiet |

## Code

- `servo.c` / `servo.h`: `servo_init()` sets LEDC timer 0 and channel 0 on GPIO 18, low-speed mode, 50 Hz, 14 bits
  (16384 steps in 20 ms, about 1.2 µs a step), duty 0 until the first angle. `servo_set_angle()` holds the angle at
  180° at most, maps 0..180° onto 500..2500 µs and turns that into duty steps (`us × 16384 / 20000`, rounded:
  90° → 1500 µs → 1229), applied from the next period (`ledc_set_duty()` into the shadow register, then
  `ledc_update_duty()`).
- `pot.c` / `pot.h`: `pot_init()` sets ADC1 oneshot on channel 0 (GPIO 1), 12 dB, 12 bits, and the curve-fitting
  calibration from eFuse. `pot_read_deg()` averages 128 readings, turns the calibrated millivolts into degrees
  (`mV × 300 / 3300`) and passes them through a 1° backlash: the reported angle moves only once the knob is more
  than 1° away from it, so it follows a turn smoothly and ignores the noise of a still knob.
- `main.c`: `app_main()` sets up the servo and the potentiometer, then loops: read the knob, cut the angle to 180°,
  set the servo, log the angle if it changed, `vTaskDelay(20 ms)`.

## Wiring

| Part | Connection |
|---|---|
| SG90 servo | orange (signal) to GPIO 18, red to the MB-102's +5 V, brown to the MB-102's GND |
| Servo supply | 5 V from the breadboard power module (MB-102), its GND joined to the board's `GND` |
| Potentiometer 10 kΩ | shaft towards you, pins down: the left pin to `GND`, the middle pin to GPIO 1, the right pin to `3V3`, so the leftmost position reads 0 V |
| USB | the board's UART port (the CH343P bridge), 115 200 baud |
| Logic analyzer (for the checks only) | CH0 on GPIO 18, its ground to `GND` |

## Checked on the board

| What | How | Result |
|---|---|---|
| The PWM | logic analyzer on GPIO 18, 4 MS/s, 0.5 s, with 90° (1500 µs) set | a period of 20.001 ms every time, the pulse 1500.25–1500.50 µs (the duty is 1229 steps = 1500.24 µs; 0.25 µs is one sample) |
| The angle to the pulse | the integer version of the formula on the Mac | 0° → 500 µs, 1° → 511, 90° → 1500, 180° → 2500, 181° and 270° → 2500 |
| The potentiometer's ends | the log while the knob turned | 0 mV at the left end; 3155 mV at the right end, which is the ADC's top, not the knob's (the rail is 3.3 V) |
| The potentiometer's travel | the knob set by eye half a turn from its left end | about 1990 mV (1985 on average over 23 s, 1933–2028): 180° × 3300 / 1990 ≈ 298°, so `POT_TRAVEL_DEG` is 300 |
| The noise, without a filter | 16 readings averaged, the knob still | 13.7 mV standard deviation (about 1.3°), 176..184° in the worst case; at a half turn the log printed 62 lines in 3.5 s (178, 179, 180 in turn) |
| The cost of 128 readings | `esp_timer_get_time()` around 100 calls | 6.8 ms a call (about 53 µs a reading) |
| The loop's period | the log's timestamps while the knob turned | 20 ms between lines: `vTaskDelay` counts 10 ms ticks and wakes at a tick, so the 6.8 ms of reading fit inside the 20 ms |
| The final code by eye and ear | the knob turned slowly from the left end to past a half turn, then left still | the shaft turns the same way as the knob, smoothly, and stops at a half turn while the knob goes on; a still knob leaves it silent |

Two other filters were tried on the board while writing it: 128 readings with a 2° hysteresis on whole degrees and a
100 ms tick kept a still knob at 73° silent for 29 s, but made a slow turn jerk in 2° steps; a moving average over 8
ticks of 20 ms with the 1° backlash was smooth. The backlash on a fractional angle stayed, and the simpler 128
readings at once replaced the moving average.

A turn down to the left end and back up (every 12th line, 240 ms apart):

```
I (13931) servo_pot: angle 93 deg from the leftmost position
I (14171) servo_pot: angle 44 deg from the leftmost position
I (15151) servo_pot: angle 3 deg from the leftmost position
I (15391) servo_pot: angle 51 deg from the leftmost position
I (15631) servo_pot: angle 112 deg from the leftmost position
I (15871) servo_pot: angle 170 deg from the leftmost position
```

## Notes

- **1:1 leaves a dead part of the knob.** The lecture spread the whole knob over the servo's 180° (about 0.6° of
  shaft for a degree of knob); 1:1 can't, so the last ~120° of the knob, about 40 % of its travel, do nothing.
  Counting both from the left puts all of it on the right; counting from the centres would leave about 60° dead at
  each end, but then 0° of the servo is not the knob's leftmost position.
- **The travel is an estimate.** The half-turn point was set by eye, and 3300 mV (the 3V3 rail) is assumed, not
  measured: the ADC can't read it. A few degrees off in either changes `POT_TRAVEL_DEG` by a few degrees.
- **The ADC stops at about 3155 mV** (as in the 3.2 homework): the knob's last ~13° (past about 287°) all read the
  same. It doesn't matter here, as it is far past 180°.
- **The ESP32's ADC noise reaches the servo.** With 16 readings the angle jumped by a few degrees on a still knob, each
  jump became a new pulse, and the log flooded. 128 readings cut the noise by about √8, and the 1° backlash stops the
  rest. The price: the reported angle can stay up to 1° behind the knob.
- **A delay is fine here.** The assignment asks for no fixed tick and doesn't forbid a delay (3.4's did).
  `vTaskDelay()` puts the task to sleep and FreeRTOS runs the others meanwhile; the lecture's `Delay_ms()` on the
  Black Pill spins the CPU instead.
- **500..2500 µs are the lecture's ends, not calibrated for this servo.** SG90s differ: a pulse past the mechanical
  stop makes the servo push against it, buzz and draw current. If it buzzes at an end, narrow `SERVO_MIN_US` or
  `SERVO_MAX_US` in `servo.c`.
- **The timing comes from the crystal.** The period measured 20.001 ms; the lecture's Black Pill ran on its internal
  RC oscillator (HSI) and measured 20.33 ms.
- **The servo needs its own 5 V** (about 100–250 mA moving, more when stalled), and the grounds must be joined, or
  GPIO 18's pulse has no reference.

## Working with the step

- **Build / Upload:** `pio run -d 01-homework-servo-pot-esp32` (add `-t upload`), or the PlatformIO buttons with this
  folder in the VS Code workspace.
- **Serial Monitor:** the UART USB-C port, 115 200 baud (set in `platformio.ini`). One program per port: close the
  monitor before flashing.
