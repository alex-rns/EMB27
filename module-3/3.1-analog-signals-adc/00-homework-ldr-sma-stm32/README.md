# 00 — Homework: an LDR night light with a simple moving average on the STM32 (Black Pill)

[← Lesson 3.1](../)

The lesson 3.1 homework (analog signals and the ADC): an LDR in a voltage divider on ADC1 is read in single-conversion
("oneshot") mode every 50 ms, a simple moving average of the last 8 readings smooths it, and two thresholds switch an
LED, so it turns on in the dark and off in the light without flickering. Built on the WeAct Black Pill
(STM32F411CEU6) instead of the ESP32 the assignment names. A superloop in C; CubeMX generates the project, our code
lives in five modules: `app` (the glue), `ldr_sensor` and `led` (the drivers), `sma` and `hysteresis` (logic with no
HAL and no pins, tested on the Mac). CubeMX's `main.c` only calls `app_init()` and `app_loop()`.
The git history of this folder builds the first version one change per commit (read the ADC, read it every 50 ms, the
moving average, the first reading filling the buffer, the LED with two thresholds), then moves it into the modules,
one module per commit.

## Assignment

Lesson 3.1 homework: Simple Moving Average (SMA).

The code and the result go to GitHub. The repository name (or the branch, or a commit message) must contain the module
number and the lesson number. The link to the repository goes into the LMS.

Task:

- Connect an LDR to the ESP32 as a voltage divider and connect its output to one of the ADC1 channels.
- Connect an LED to any available GPIO.
- Read the ADC values in oneshot mode.
- Implement a Simple Moving Average (SMA) to filter the light readings.
- Using the filtered data:
  - when it gets dark, the LED turns on;
  - when it gets light, the LED turns off.
- The LED must not flicker with small changes of light.
- Implement two thresholds (hysteresis) to prevent the flickering.

## How each point is done

| Point | Where | How |
|---|---|---|
| The LDR divider on an ADC1 channel | wiring, `.ioc` | 3V3 → LDR → PA1 → 10 kΩ → GND; PA1 = ADC1_IN1 (the F411 has one ADC, ADC1) |
| The LED on a GPIO | wiring, `led.c` | PB8 → 220 Ω → LED → GND; `led_set(true / false)` |
| Oneshot reading | `ldr_sensor.c`, `app.c` | single conversion, software start, polled: `HAL_ADC_Start` → `HAL_ADC_PollForConversion` → `HAL_ADC_GetValue`, one conversion per call, every 50 ms (`LDR_SAMPLE_MS`). The STM32 counterpart of ESP-IDF's oneshot mode |
| The SMA | `sma.c` | the last 8 readings (`LDR_AVG_SAMPLES`, 0.4 s) in a ring buffer with a running sum; the first reading fills the whole buffer |
| Dark: on, light: off | `app.c` → `hysteresis.c` → `led.c` | the average decides, not the raw reading |
| No flicker, two thresholds | `hysteresis.c`, `app.c` | on below 2000, off above 2600 (`LED_ON_BELOW`, `LED_OFF_ABOVE`); in between the LED keeps its state; `_Static_assert` keeps the two in order |
| Module and lesson in the name | git | the folder `module-3/3.1-analog-signals-adc/`, the branch `module-3/3.1-analog-signals-adc-homework`, every commit message starts with `module-3/3.1-analog-signals-adc:` |

## Architecture

```
 ┌────────────────────────────────────────────────────────────────────┐
 │ main.c (CubeMX): the inits, app_init(), app_loop() in while (1)    │
 └──────────────────────────────────┬─────────────────────────────────┘
                                    │
 ┌──────────────────────────────────▼─────────────────────────────────┐
 │ app.c: ldr_slot() every 50 ms (HAL_GetTick): one reading,          │
 │ the new average, the decision, the LED                             │
 └───────┬─────────────────┬─────────────────┬─────────────────┬──────┘
         │                 │                 │                 │
 ┌───────▼──────┐  ┌───────▼──────┐  ┌───────▼──────┐  ┌───────▼──────┐
 │ ldr_sensor   │  │ sma          │  │ hysteresis   │  │ led          │
 │ • ADC1, PA1  │  │ • average of │  │ • two        │  │ • PB8, lit   │
 │ • one        │  │   the last 8 │  │   thresholds │  │   on HIGH    │
 │   conversion │  │ • no HAL,    │  │ • no HAL,    │  │              │
 │              │  │   no pins    │  │   no pins    │  │              │
 └──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
```

No module includes another one; only `app.c` connects them. `sma` and `hysteresis` keep their state in a struct the
caller allocates (`sma_t`, `hysteresis_t`; the average's buffer is the caller's too) and know nothing about the ADC,
the light or the LED: the same code would average a temperature or run a thermostat, and both compile and run on the
Mac. `ldr_sensor` and `led` are the only code that touches the hardware: the ADC handle and the pin with its active
level. `app.c` splits its slot in two: `period_elapsed()` decides when (the wrap-safe `now - last >= period` check on
`HAL_GetTick()`), `ldr_handle_reading()` what one good reading does. A failed conversion is skipped, so the average
keeps the older readings.

## Project

| Pin | CubeMX setting | Used for |
|---|---|---|
| PA1 | ADC1_IN1; label `LDR` | the divider's midpoint |
| PB8 | GPIO output, push-pull, low at reset; label `LED` | 220 Ω → LED → GND |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |
| PH0 / PH1 | RCC: HSE crystal | 25 MHz crystal |

**Clock:** HSE 25 MHz → PLL (M = 12, N = 96, P = 2) → SYSCLK 100 MHz; APB1 50 MHz, APB2 100 MHz. **ADC1:** PCLK2 / 4
= 25 MHz (the datasheet allows 36 MHz at most, and / 2 would give 50), 12 bits, right-aligned, one conversion per
software start, channel 1 sampled for 84 cycles: 84 + 12 = 96 ADC clocks = 3.84 µs per conversion.
STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

Why 84 cycles: the divider's output resistance is at most 10 kΩ (the LDR and the 10 kΩ in parallel), and charging the
ADC's sample capacitor to ¼ LSB through it takes about 1.1 µs. 28 cycles (1.12 µs) would be right at that edge, 84
cycles (3.36 µs) leave three times the time; the datasheet's formula allows up to about 43 kΩ at 84 cycles and 25 MHz.

The pins avoid PA0 (the board's KEY button) and PA4–PA7 (the board's SPI flash).

## Wiring

![Schematic](schematic.svg)

| Part | Connection |
|---|---|
| LDR divider | the Black Pill's `3V3` → LDR → PA1 → 10 kΩ → `G` (GND) |
| LED | PB8 → 220 Ω → LED anode; cathode → `G` |
| SWD header | ST-Link V2: 3.3V, SWDIO, SWCLK, GND; the board runs from the ST-Link |

More light → lower LDR resistance → higher voltage at PA1 → higher reading.

## Notes

- The divider is fed from the board's own `3V3`, which is also the ADC's reference (on this 48-pin package VREF+ is
  tied to VDDA, the 3.3 V rail). The reading is a ratio of the two, so it doesn't depend on the exact rail voltage.
  A divider fed from another supply would read differently.
- The first reading fills the whole buffer. Starting from zeros, the average would climb 411, 822, … for 0.4 s
  after every reset, and in a lit room the LED would flash at every power-up.
- The thresholds are for this breadboard and this room: room light gave an average of about 3290, a finger over the
  LDR about 1620–1730 (a finger lets some light through: the LDR was about 13–15 kΩ, not the hundreds of kΩ of real
  darkness). Read `s_ldr_avg` in your own light and dark before changing them.
- The moving average delays: a slow change comes out (N - 1) × Ts / 2 = 175 ms late, a sudden one takes
  8 × 50 ms = 0.4 s to come through in full. More samples smooth more and react later.
- The integer average rounds down: 26 303 / 8 gives 3287.
- `ldr_sensor_read()` blocks for one conversion, 3.84 µs. Its timeout (`LDR_ADC_TIMEOUT_MS`, 1) only guards a hung
  ADC; HAL compares `> Timeout` on a 1 ms tick, so that worst case is 1 to 2 ms.
- Never compare a reading with `==`: noise moves it by several codes. The thresholds use `<` and `>`.
- PA0 is no place for the LDR on this board: pressing KEY pulls it to GND through 330 Ω.

## Checked on the board

Values read from RAM over the ST-Link while the program runs (`s_ldr_raw`, `s_ldr_avg`, the buffer, the
`hysteresis_t`) and the LED pin from `GPIOB->ODR`:

| Check | Result |
|---|---|
| Room light | raw 3224–3302, average 3225–3301 |
| A finger over the LDR | raw 1604–1779, average 1617–1734: the raw readings wander by tens of codes, the average by a few |
| Timing | the last read time was always a multiple of 50 ms, and less than 50 ms behind `HAL_GetTick()` (5 snapshots) |
| Start | 147 ms after a reset (two readings) the average was 3293; with a buffer starting at zeros it would have been (3294 + 3292) / 8 = 823 |
| The LED | off in room light (`ODR` bit 8 = 0), on under a finger (bit 8 = 1); no flicker while a hand moves over the LDR |
| The refactor | after each module commit the board was checked again: the readings, the average and the LED in room light; the LED under a finger after the `led`, `hysteresis` and `app` commits |

On the Mac, `sma.c` and `hysteresis.c` compiled with `clang -Wall -Wextra -Wconversion` (0 warnings):

- `sma`: 100 000 random readings plus 0 and 4095 give the same averages as the code before the refactor; a step
  100 → 200 with 4 samples gives 125, 150, 175, 200; the first reading 3294 gives 3294 at once.
- `hysteresis`: 200 000 values of a random walk, thresholds included, give the same decisions as the code before the
  refactor; the lecture's slide 33 sequence (thresholds 1500 / 1800: 1900, 1490, 1510, 1750, 1800, 1810, 1700) gives
  OFF, ON, ON, ON, ON, OFF, OFF, as on the slide.

The build: `.github/scripts/build-stm32.sh` (CubeIDE's GCC 14.3 with `-Wextra`) gives 0 warnings; cppcheck with all
checks on our six files reports two style notes, both on CubeMX's own functions (`SystemClock_Config`,
`Error_Handler`).

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder (opened in place, not copied into the
  workspace).
- **Build:** Project → Build Project (the hammer). After a pull that adds `.c` files, refresh the project (F5) first:
  CubeIDE writes its `Debug/` makefiles from the files it has seen.
- **Flash:** Run → Run As → STM32 C/C++ Application, with the ST-Link connected. Launch files (`*.launch`) hold
  absolute paths and are not committed, so the first Run creates one.
- **Change pins or peripherals** in `00-homework-ldr-sma-stm32.ioc` with CubeMX, then regenerate: everything in
  CubeMX's files outside the `USER CODE` blocks is rewritten.
