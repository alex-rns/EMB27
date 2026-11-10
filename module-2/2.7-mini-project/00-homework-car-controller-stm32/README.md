# 00 — Homework: a car body controller on the STM32 (Black Pill)

The lesson 2.7 mini project: project 2 of the three offered, the automotive Body Control Module (central lock, turn
signals, hazard lights, crash sensor), built on the WeAct Black Pill (STM32F411CEU6) instead of the ESP32-S3 the
course PDF uses. A superloop in C without FreeRTOS; CubeMX generates the project, our code lives in three modules
(`button_fsm`, `load_driver`, `pot_sensor`) and in the `USER CODE` blocks of `main.c`, which glues them together.
The git history of this folder follows the PDF's steps one change per commit.

## Assignment

"Project 4 · Automotive · ~7 h · ESP-IDF v5.5.4. Body control module: central locking, turn signals and hazard
lights" (the course PDF; its goal list, challenges and final checklist translated in full; the 12 steps in between
are a guided tutorial with ESP-IDF code, listed by title).

A step-by-step practice: build on the ESP32-S3 (plain ESP-IDF, no Arduino) a real body "on-board computer" (Body
Control Module, BCM), the same class of device that controls the door locks, turn signals and hazard lights in a real
car. Four buttons run through the finite state machine (FSM) of module 2.6, each telling a short press from a long
one. The "crash sensor" (the emergency button) is wired as a real hardware interrupt (modules 2.3–2.4), not polled:
a fundamental difference you will see on the logic analyzer. The relay and the buzzer are switched through a BC547B
transistor (module 2.2): no load goes straight to a GPIO. All time in the system, from the debounce to the blink rate,
is controlled by the `esp_timer` hardware timers (module 2.5). The code is C in a style fit for a microcontroller
(module 2.1): static memory, enum/struct, no `malloc`, no `String`. It integrates modules 2.1 (C++ for MCUs), 2.2
(passive/active loads), 2.3–2.4 (hardware interrupts), 2.5 (timers) and 2.6 (FSM debounce).

**The goal.** At the end you will have:

1. A working ESP-IDF project (`idf.py build / flash / monitor`) with a proper modular architecture: separate
   components (`components/…`), not one 1000-line `main.c`.
2. Four buttons, each its own instance of the debounce FSM (IDLE → PRESSED → HELD → RELEASE), telling a short
   (< 2 s) press from a long (≥ 2 s) one and never missing a press, even one shorter than 10 ms.
3. One button (the "crash sensor") working through a real GPIO hardware interrupt (`gpio_isr_handler_add`), not
   polling, and a clear understanding of why an interrupt is mandatory, not an option, for time-critical events
   (crash sensor, airbag).
4. Two independent periodic `esp_timer`s, one ticking the FSM every 10 ms, the other reading the potentiometer every
   50 ms: proof that ESP-IDF can run several independent timers without any `delay()` that blocks the system.
5. A power load driver on a BC547B: the relay (door lock / emergency circuit) and the buzzer (sound signals), with the
   base resistor calculation and an explanation of why a GPIO must never drive a relay coil directly.
6. Logic analyzer captures: the "dirty" bounce at a button input and the clean, predictable edge of the system's
   reaction (relay/buzzer): proof that the debounce works.
7. A fully working scenario: the door lock (short = lock/unlock, long = valet mode), the turn signal (short = on/off,
   long = 3 lane-change blinks), the hazard lights (short = on/off, long = diagnostics of all systems), the crash
   sensor (instantly, through an interrupt, turns on the hazard lights and a relay pulse).

**The steps:** 0 hardware, wiring, safety · 1 initializing the ESP-IDF project · 2 modular architecture: components
instead of "one file" · 3 C for MCUs: types, enum, static memory · 4 the load driver: relay and buzzer through
BC547B · 5 buttons: raw read and bounce on the analyzer · 6 hardware interrupt: the crash sensor · 7 `esp_timer`: the
FSM tick (10 ms) and the ADC read (50 ms) · 8 the full debounce FSM of a button (IDLE / PRESSED / HELD / RELEASE) ·
9 the application logic: lock, turn signals, hazard lights · 10 potentiometer → blink speed · 11 the final
architecture · 12 logs and diagnostics. Steps 1–12 each end with a mini task.

**Challenges** (optional):

1. A fifth FSM state, double press: add `BTN_STATE_WAIT_DOUBLE` between RELEASE and the return to IDLE; a new press
   within ~300 ms after the release is a double click (e.g. the "3 lane-change blinks" of step 9 for the turn signal).
2. A button combination (LOCK + HAZARD at once): detect "both held together ≥ 1 s" as a separate condition on top of
   the existing FSMs (check the state of both `button_fsm_t` in `app_fsm_tick_all`): the classic car combination to
   enter a service menu.
3. Protection against a "stuck" crash sensor: if the crash ISR fires more than 5 times a second (a faulty contact,
   interference), disable it with `gpio_isr_handler_remove()` for 2 seconds and log it with `ESP_LOGE`. Where to
   count the rate, in the ISR or in `crash_task`? Why?
4. Debounce calibrated per button: measure the real bounce of each of the 4 buttons on the analyzer (it always differs
   a little) and make `BTN_DEBOUNCE_MS` a field of `button_fsm_t` instead of a global constant, with the smallest safe
   value for each button.
5. An event counter in NVS: keep "how many times the doors were locked" across reboots with `nvs_flash` (built
   into ESP-IDF): a first look at non-volatile storage on the ESP32-S3.

**What you should have at the end:**

- `idf.py build / flash / monitor` pass without errors for esp32s3.
- A modular structure: 3 components + main, each with its own `CMakeLists.txt`, no business logic outside main.
- 3 buttons go correctly through the full FSM cycle IDLE → PRESSED → HELD → RELEASE → IDLE, telling short from long,
  confirmed on the logic analyzer (bounce at the input vs one clean output pulse).
- The 4th button (the crash sensor) reacts through a real GPIO hardware interrupt handed over to a task through a
  queue, and you can explain why heavy work must not be done right in the ISR.
- Two independent `esp_timer`s (10 ms FSM tick, 50 ms ADC) run at the same time, without blocking each other and
  without any `delay()` that freezes the system.
- The relay and the buzzer are driven safely (a transistor switch where needed; awareness of the flyback diode's role
  even where there is none physically).
- The potentiometer smoothly sets the blink rate of the turn signals / hazard lights.
- You can draw the component dependency diagram on paper and explain why it is built this way, not as one file.

## What differs from the PDF

The PDF is written for the ESP32-S3 with ESP-IDF and FreeRTOS. This version keeps its structure, its FSM and its
logic, and changes what the board, the kit or the topics covered so far require:

| PDF | Here | Why |
|---|---|---|
| ESP32-S3, ESP-IDF, `idf.py` | Black Pill STM32F411CE, CubeMX + HAL, STM32CubeIDE | the author's choice of board for this project |
| FreeRTOS tasks, two `esp_timer`s, a queue | one superloop: `HAL_GetTick()` slots every 10 ms (buttons), 50 ms (pot) and half a blink period (flasher) | no RTOS yet in the course; the slots never wait, so nothing blocks |
| crash ISR → queue → `crash_task` → callback | EXTI interrupt → `volatile` flag → `crash_sensor_take()` in the superloop → `handle_crash()` in `main.c` | the same split (the ISR only signals, the reaction runs outside it) without a task; `button_fsm` still knows nothing about the loads |
| internal pull-down, pressed = HIGH, interrupt on the rising edge | internal pull-up, pressed = LOW, interrupt on the falling edge | the usual STM32 wiring (button to GND); `raw_pressed()` hides it, as the PDF intends. In the toy car the crash button needs only two wires, PA8 and GND |
| 3 FSM buttons (LOCK, TURN_L, HAZARD) + crash; no LEDs | 4 FSM buttons (+ RIGHT) + crash, 4 turn LEDs (left pair, right pair) | the author's addition: the controller sits in a wooden toy car, the LEDs at its corners |
| active buzzer through a BC547B | passive buzzer on TIM3 CH4 PWM (PB1 → 220 Ω → buzzer → GND, no transistor) | the active one was too loud; the passive one plays a soft 800 Hz beep and also clicks like a flasher relay |
| relay module IN straight from a 3.3 V GPIO | relay module through a BC547B with a 10 kΩ pull-up to 5 V, the module fed with 5 V from the breadboard supply | this kit's module has a 5 V coil and a PNP input stage that a 3.3 V high doesn't turn off (as in lesson 2.2) |
| logs over UART, step 12 (log levels), a status line every second | no UART: the state shows on the LEDs and the relay, values were read over SWD; step 12 dropped | UART logs were not needed for any check here |
| unlock beep 40 ms | unlock = two 60 ms beeps | the author's change: like a real car (two 40 ms beeps would merge into one) |
| after a relay pulse (diagnostics, crash) the relay is switched off | after a pulse the relay goes back to the lock level | in the PDF a pulse while locked leaves the relay off while the lock flag still says locked |
| `load_buzzer_chirp()` blocks (`vTaskDelay`) | `load_buzzer_chirp()` / `load_relay_pulse()` start and return; `load_driver_update()` ends them | a blocking beep would stop the superloop and every button with it |

Not done: the mini tasks of every step and the logic analyzer measurements (the author's choice), the challenges.

## How each point is done

| Goal / final check | Where |
|---|---|
| 1. Modules, no 1000-line `main.c` | `Core/Inc` + `Core/Src`: `button_fsm`, `load_driver`, `pot_sensor`; `main.c` holds only the glue (the state flags and four small handlers) |
| 2. Four FSM buttons, short < 2 s / long ≥ 2 s | `button_fsm_tick()` in `button_fsm.c`: the PDF's 4-state FSM, 50 ms debounce on press and release, `BTN_EVENT_LONG_PRESS` once while still held; `s_buttons[4]` in `main.c`, ticked every 10 ms |
| 3. Crash sensor on a hardware interrupt | PA8 = EXTI line 8 (CubeMX), `HAL_GPIO_EXTI_Callback()` in `button_fsm.c` only sets `s_crash_flag`; `crash_sensor_take()` hands it to the superloop once |
| 4. Two periodic "timers" without blocking | the 10 ms and 50 ms slots in the superloop (`now - last >= period`), no `HAL_Delay` anywhere |
| 5. Relay and buzzer driver | `load_driver.c`: the relay through the BC547B stage, the buzzer through TIM3 PWM (see Notes for why no transistor is needed there) |
| 6. Analyzer captures | not done (see above); the debounce was checked by eye: one press = one toggle |
| 7. The scenario | `handle_lock_button()`, `handle_turn_button()`, `handle_hazard_button()`, `handle_crash()` in `main.c`; the table below |
| The pot sets the blink rate | `pot_sensor.c` (ADC1, PA1); in `main.c` period = 200 + raw × 600 / 4095 ms |
| The dependency diagram | [Architecture](#architecture) |

### What each button does

| Button | Short press | Long press (≥ 2 s, fires while still held) |
|---|---|---|
| LOCK | lock: relay on, one 80 ms beep; unlock: relay off, two 60 ms beeps | valet mode on/off (the onboard LED on PC13 shows it), 300 ms beep |
| LEFT | left turn signal on/off: the left pair blinks with a tick-tock; switches the right one off | nothing (the lane change is challenge 1) |
| RIGHT | the same for the right side | nothing |
| HAZARD | hazard lights on/off: all 4 blink; a turn signal that was on comes back when the hazard goes off | diagnostics: relay pulse 200 ms + 200 ms beep |
| CRASH (interrupt) | the hazard lights forced on, relay pulse 150 ms, 150 ms beep | — |

The decisions the PDF leaves open (it has one turn signal) were the author's: one side at a time, and the hazard
lights win while on without clearing the turn signal. The valet mini task (a short LOCK ignored in valet mode) was
not added, so valet only toggles its flag, its LED and the beep. While locked, the relay is already on, so a
diagnostics or crash pulse doesn't click it (the author kept it that way).

## Architecture

```
                    ┌────────────────────────────────────────┐
                    │ main.c: the glue (all the car's logic) │
                    │ superloop, HAL_GetTick() slots         │
                    └───┬───────────────┬────────────────┬───┘
                        │               │                │
      ┌─────────────────▼───┐   ┌───────▼───────┐   ┌────▼─────────────────┐
      │ button_fsm          │   │ pot_sensor    │   │ load_driver          │
      │ • 4 FSMs, 10 ms     │   │ • ADC1, PA1   │   │ • relay PB0 (BC547B) │
      │ • CRASH: EXTI, PA8  │   │ • every 50 ms │   │ • buzzer PB1 (TIM3)  │
      │   → a flag          │   │               │   │ • LEDs PB8 / PB9     │
      └─────────────────────┘   └───────────────┘   └──────────────────────┘
```

No module includes another one: each sees only its own header and CubeMX's `main.h` (the pin names). `button_fsm`
and `load_driver` don't know each other; only `main.c` connects a button event to a load. One clock, `HAL_GetTick()`
(SysTick, 1 ms), feeds every slot; the crash interrupt doesn't wait for any of them.

## Project

| Pin | CubeMX setting | Used for |
|---|---|---|
| PB0 | GPIO output, low at reset; label `RELAY` | 10 kΩ → BC547B base (the relay stage) |
| PB1 | TIM3_CH4 (AF2); label `BUZZER` | 220 Ω → passive buzzer → GND |
| PB8 / PB9 | GPIO outputs, low at reset; labels `LED_LEFT` / `LED_RIGHT` | two LEDs each, each with its own 220 Ω to GND |
| PB12 / PB13 / PB14 / PB15 | GPIO inputs, pull-up; labels `BTN_LOCK` / `BTN_LEFT` / `BTN_RIGHT` / `BTN_HAZARD` | buttons to GND |
| PA8 | GPIO_EXTI8, falling edge, pull-up; label `CRASH`; NVIC: EXTI line[9:5] enabled | the crash button to GND |
| PA1 | ADC1_IN1; label `POT` | the potentiometer's wiper |
| PC13 | GPIO output; label `LED` | the onboard LED (lit on LOW): valet mode |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |
| PH0 / PH1 | RCC: HSE crystal | 25 MHz crystal |

**Clock:** HSE 25 MHz → PLL (M = 12, N = 96, P = 2) → SYSCLK 100 MHz; APB1 50 MHz (its timers 100 MHz), APB2
100 MHz. **TIM3:** prescaler 99 → 1 MHz, period 1249 → 800 Hz, PWM mode 1, CCR4 0 at start (silent). **ADC1:**
PCLK2 / 4 = 25 MHz (the datasheet allows 36 MHz at most), 12 bits, one conversion per software start, channel 1
sampled for 84 cycles. STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

The pins avoid PA4–PA7 (the board's SPI flash), PB2 (BOOT1) and PB11 (not bonded on this package).

## Wiring

![Schematic](schematic.svg)

| Part | Connection |
|---|---|
| Relay stage | PB0 → 10 kΩ → BC547B base; emitter → GND; collector → the module's `IN`, which also has a 10 kΩ pull-up to +5 V; module `VCC` → +5 V, `GND` → GND |
| +5 V | the breadboard supply (MB-102); its GND is shared with the Black Pill's `G` |
| Buzzer | PB1 → 220 Ω → passive buzzer → GND |
| Turn LEDs | PB8 → 220 Ω → LED → GND, twice (left front, left rear); PB9 the same for the right side |
| Buttons | PB12, PB13, PB14, PB15 and PA8, each to GND through a tactile button, on two diagonal legs |
| Potentiometer 10 kΩ | the ends to the Black Pill's `3V3` and GND, the wiper to PA1 |
| SWD header | ST-Link V2: 3.3V, SWDIO, SWCLK, GND; the board runs from the ST-Link |

The Black Pill's `5V` pin is dead while the board runs from the ST-Link (it comes from the board's own USB-C through
a diode), hence the breadboard supply for the relay.

## Notes

- **The transistor's legs.** With the flat side facing, this kit's BC547B stage only worked with the outer legs
  swapped against the datasheet's C-B-E order (lesson 2.2 saw the same). Reversed, it still conducted a little:
  base 0 ↔ 0.54 V, but the module's `IN` only fell from 5.21 to 4.56 V and the relay stayed off.
- **Why the buzzer needs no transistor.** At most 3.3 V / 220 Ω = 15 mA flows from PB1, within a pin's limit; the
  relay coil needs far more, and 5 V.
- **A passive buzzer clicks, an active one beeps.** The passive one only moves when its voltage changes: PWM at
  800 Hz (CCR = 156, about 1/8 of the period, for a soft tone) makes it beep, and holding the pin steady HIGH
  (CCR = ARR + 1) or LOW (CCR = 0) makes one click per change, the flasher's tick-tock.
- **Nothing in the interrupt but the flag.** CubeMX gives EXTI9_5 priority 0, higher than SysTick's 15: while the
  callback runs, `HAL_GetTick()` stands still, so a `HAL_Delay()` inside it would never end.
- **`HAL_GPIO_EXTI_Callback` replaces a weak function.** HAL defines an empty `__weak` version; ours with the same
  name and signature wins at link time (`nm` shows it as `T`, a strong symbol). A typo in the name would leave the
  empty one in place without any error.
- **The crash flag is read and cleared in two steps.** A second crash between the two merges with the first one: the
  reaction is the same, so no locking is needed.
- **`now - start >= len` on `uint32_t`** survives the wrap of `HAL_GetTick()` after about 49.7 days.
  `load_driver_update()` reads its own `HAL_GetTick()`: with the loop's `now`, a beep started later in the same pass
  would have `start > now`, and `now - start` would wrap to about 4.3 billion and end the beep at once.
- **The ADC needs time to sample.** Its internal capacitor charges through the source: a 10 kΩ potentiometer is up to
  2.5 kΩ in the middle. With the datasheet's formula (Table 65, Equation 1) 3 cycles are not enough even for a 0 Ω
  source, 84 cycles allow about 43 kΩ. One conversion then takes about 4 µs, so polling it is fine.
- **The pot must sit on 3.3 V,** not on the breadboard's 5 V rail: an analog pin is not 5 V tolerant.
- **`volatile` only where an interrupt writes.** `s_crash_flag` is written in the interrupt and read in the loop;
  the pot value is written and read by the loop only, so it needs none.
- **Two button legs are always connected.** A button whose pin reads 0 all the time sits on the legs of one pair or
  shares a row with GND; diagonal legs are always on different pairs.

## Checked on the board

- **Outputs:** the relay clicked on and off every 500 ms with a test loop (after the transistor fix above); both LED
  pairs alternated every 500 ms; the buzzer beeps.
- **Buttons:** each of the five read 1 when released and 0 while held, logged through the ST-Link (`GPIOB IDR`,
  `GPIOA IDR`); the PA8 interrupt set up as expected (`EXTI IMR8` = 1, `FTSR8` = 1, `RTSR8` = 0, `SYSCFG EXTICR3` =
  port A).
- **Pot:** `s_last_raw` read over SWD while turning the knob: smooth from 0 to 4095 and back, three sweeps.
- **FSM (step 8 test):** every short press toggled the onboard LED once; a 3 s hold toggled the turn LEDs after about
  2 s, while still held, and the release gave no short press.
- **The scenario:** every row of the button table above, the flasher speed from fast (200 ms) to slow (800 ms) with
  the knob.

Build (Debug, `-O0 -g3`): 0 errors, 0 warnings with `-Wall -Wextra` (CI's `build-stm32.sh`). `arm-none-eabi-size`:
`text` 13008, `data` 12, `bss` 1812 bytes.

Commit 0573a91 (the CubeMX setup of PA8) doesn't build: a word typed into the editor by accident landed in `main.c`;
0a9d29a removes it.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder (opened in place, not copied into the
  workspace).
- **Build:** Project → Build Project (the hammer). After a pull that adds `.c` files, refresh the project (F5) first:
  CubeIDE writes its `Debug/` makefiles from the files it has seen.
- **Flash:** Run → Run As → STM32 C/C++ Application, with the ST-Link connected. Launch files (`*.launch`) hold
  absolute paths and are not committed, so the first Run creates one.
- **Change pins or peripherals** in `00-homework-car-controller-stm32.ioc` with CubeMX, then regenerate: everything
  in CubeMX's files outside the `USER CODE` blocks is rewritten.
