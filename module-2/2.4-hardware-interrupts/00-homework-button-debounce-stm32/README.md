# 00 — Homework: button debounce, short and long press on the STM32 (Black Pill)

The lesson 2.4 homework on the WeAct Black Pill (STM32F411CEU6). A button on PB5 raises an EXTI interrupt on both
edges, press and release; the interrupt only records that an edge came and when. The main loop waits until 30 ms have
passed since the last edge, then reads PB5: low means a press, and the loop notes its time; high means a release, and
the hold time decides: under 1 s toggles the PB6 LED (short press), 1 s or more toggles the board's blue LED on PC13
(long press). Nothing happens at the press itself. A PB7 LED blinks as a heartbeat. Written in C inside the
`USER CODE` blocks of CubeMX's `main.c`, starting from the lecture's code.

This version covers task 3 of the assignment and the extra assignment the teacher gave for the STM32. Tasks 1, 2, 4,
5 and 6 belong to the ESP32 version (the assignment asks for the Arduino framework or ESP-IDF). The commits follow
the work: task 3 first, then the extra tasks one by one.

## Assignment

The teacher's text in the LMS, translated in full.

**Homework: removing button contact bounce**

Code and the result on GitHub; the link to the repository in the LMS. Use the Arduino framework or ESP-IDF.

**Task 1: Basic implementation (no debounce, done in class)**
- Set up a GPIO interrupt on FALLING.
- On every interrupt: increment a counter; print the counter to the log.

Expected result: one physical press gives several interrupts; the interrupt also fires when the button is released.

**Task 2: Software debounce with a timer (time-based)**
- Add a time-based debounce: ignore an interrupt if less than 50 ms have passed since the previous one.
- Implement it outside the ISR.

Expected result: fewer false triggers; triggers on release are still possible.

**Task 3: Debounce by checking the level (state-based)**
- The interrupt only signals "an event".
- In `loop()` / a task: accept the event only if the button is still pressed; ignore events on release.

Expected result: exactly one reaction to one press; a release causes no action.

**Task 4: Polling + debounce (no interrupts)**
- Remove the interrupt completely.
- Poll the button every 5–10 ms.
- Implement the debounce as a state machine.

Expected result: the most stable behaviour; a slightly longer reaction delay.

**Task 5: Hardware debounce**
- Add an RC filter (a 100 nF capacitor and a 100 Ω resistor, the 10 kΩ pull-up unchanged).
- Repeat parts 1–4.
- Compare the results with and without the RC.

**Task 6: Comparison table**

| Method | Number of false triggers | Delay | Complexity | Comment |
|---|---|---|---|---|
| No debounce | | | | |
| Time-based | | | | |
| State-based | | | | |
| Polling | | | | |
| Hardware RC | | | | |

**Extra homework (for the STM32)**

Using EXTI interrupts, implement different kinds of button handling:
1. On a short press.
2. On a long press.
3. The logic (e.g. an LED) fires after the button is released, not at the moment of the press.

## How each point is done

All in `Core/Src/main.c`.

| Task | In this version |
|---|---|
| 1. No debounce | not here: the ESP32 version |
| 2. Time-based | not here: the ESP32 version |
| 3. State-based | `HAL_GPIO_EXTI_Callback()` only sets `btn_pressed_flag` and stores `btn_pressed_timestamp = HAL_GetTick()`. The `while (1)` loop acts once the flag is set and 30 ms have passed since the last edge: it reads PB5 with `HAL_GPIO_ReadPin()` and clears the flag either way. In the task 3 commit (`3f68f4e`) it toggled PB6 if PB5 was still low (pressed) and dropped a release; the extra task 3 moved the action to the release, so the final code acts on a confirmed release instead |
| 4. Polling | not here: the ESP32 version |
| 5. Hardware RC | not here |
| 6. Table | not here |

| Extra task | In this version |
|---|---|
| 3. Logic after the release | PB5's EXTI fires on both edges (CubeMX `GPIO_MODE_IT_RISING_FALLING`: bit 5 set in both `EXTI_RTSR` and `EXTI_FTSR`). 30 ms after the last edge the loop reads PB5: high (`GPIO_PIN_SET`) means the button was released, and only that branch toggles an LED. A confirmed press only stores the time |
| 1. Short press | on a confirmed press the loop stores `press_start = now`; on the release, `now - press_start < 1000u` toggles PB6 (`LED_1`) |
| 2. Long press | otherwise (held 1 s or more) the release toggles the blue LED on PC13 (`LED_3`) |

The assignment gives no thresholds: 1 s between short and long is our choice. Both ends of a press are confirmed
about 30 ms after their last edge, so the two delays cancel in `now - press_start`.

How the interrupt reaches our code: PB5 is EXTI line 5, which shares the `EXTI9_5_IRQn` vector with lines 6–9.
CubeMX generates `EXTI9_5_IRQHandler()` in `stm32f4xx_it.c`, which calls `HAL_GPIO_EXTI_IRQHandler(BTN_Pin)`; the HAL
checks line 5's pending bit, clears it, then calls our `HAL_GPIO_EXTI_Callback(GPIO_Pin)`. With both edges enabled,
the release and its bounce call it too.

The bounce timing works out because every bounce edge calls the callback again and overwrites the timestamp, so the
30 ms count from the last edge, after the contacts have settled.

## Project

A CubeIDE project generated by CubeMX into this folder; our code lives only in the `USER CODE` blocks of
`Core/Src/main.c`, which a regenerate keeps.

| Pin | CubeMX setting | Used for |
|---|---|---|
| PB5 | `GPIO_EXTI5`: external interrupt on the rising and falling edges, pull-up; label `BTN` | button to GND |
| PB6 | GPIO output, push-pull, no pull; label `LED_1` | LED toggled by a short press |
| PB7 | GPIO output, push-pull, no pull; label `LED_2` | heartbeat LED, toggles every 500 ms |
| PC13 | GPIO output, push-pull, no pull, output level High; label `LED_3` | the board's blue LED, toggled by a long press |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |
| PH0 / PH1 | RCC: HSE crystal | 25 MHz crystal |

**NVIC:** EXTI line[9:5] interrupts enabled, preemption priority 1; SysTick stays at 15, the lowest.
**Clock:** HSE 25 MHz → PLL (M = 12, N = 96, P = 2) → SYSCLK 100 MHz; time from SysTick's 1 ms `HAL_GetTick()`.
STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

## Wiring

The lecture's stand:

| Part | Connection |
|---|---|
| Button | PB5 and the breadboard's GND rail, on two diagonal legs |
| LED 1 | PB6 → series resistor (the lecture's stand: 330 Ω) → LED anode (long leg); the cathode → GND rail |
| LED 2 | PB7, the same |
| LED 3 | on the board, no wiring: 3.3 V → 1.5 kΩ (R5) → blue LED → PC13 (WeAct V3.1 schematic) |
| Black Pill `G` | the breadboard's GND rail |
| SWD header | ST-Link V2: 3.3V, SWDIO, SWCLK, GND; the board runs from the ST-Link |

PB5 reads 1 while released (internal pull-up) and 0 while pressed.

## Notes

- **PB5 is a 3.3 V-only pin.** The datasheet's pinout table marks it `TC` (standard 3.3 V I/O), while PB6 and PB7 are
  `FT` (5 V tolerant): never wire the button to 5 V.
- **The blue LED is lit on low.** Its anode sits on 3.3 V, so PC13 low lights it. CubeMX's output level High makes
  `MX_GPIO_Init()` write PC13 high before it turns the pin into an output, so the LED starts off without a flash.
- **`GPIO_Pin` is a mask.** The callback gets `GPIO_PIN_5` (`0x0020`), not the number 5; compare it with `BTN_Pin`.
  `GPIO_Pin == 5` compiles and never matches.
- **No waiting in the callback.** It runs inside the interrupt at priority 1, above SysTick's 15, so SysTick can't
  run while it does: `HAL_GetTick()` there just reads the counter, but a `HAL_Delay()` would wait for a tick that
  never comes (from the code; not tried on the board).
- **The level is checked once,** at the 30 ms mark, not throughout the 30 ms: a simplified "verify press".
- **A tap shorter than 30 ms counts as long (known limit, from the code; not seen on the board).** If the contacts
  close for less than 30 ms, the press is never confirmed but the release is, and the hold time is counted from the
  previous press, usually over 1 s. A flag set on a confirmed press and required on the release would drop such a tap.
- **Two rare races (known limits).** If an edge arrives between `uint32_t now = HAL_GetTick();` and the check, and
  the tick advances in between, `now - btn_pressed_timestamp` wraps to about 4.29 billion and the edge is handled at
  once, without the 30 ms wait. An edge that arrives between the check and `btn_pressed_flag = 0u` is cleared with the
  flag. Both windows last microseconds. The cure would be a short critical section (`__disable_irq()` /
  `__enable_irq()`) that reads the timestamp first and checks and clears the flag as one step.
- **No log on this board.** The ST-Link V2 has no serial port; values here are read over the ST-Link (CubeIDE's
  Live Expressions while debugging, or a memory read).

## Checked on the board

- **Task 3 (`3f68f4e`):** each press toggled PB6 once, releasing the button did nothing, PB7 blinked (500 ms on,
  500 ms off). Registers read over the ST-Link while the program ran: `EXTI_IMR` = `0x20` and `EXTI_FTSR` = `0x20`
  (line 5 enabled, falling edge), `EXTI_PR` = 0, `GPIOB` `IDR5` = 1 with the button released.
- **Both edges (`98fbf6c`):** `EXTI_RTSR` = `EXTI_FTSR` = `0x20`, so line 5 fires on the release too; PB6 still
  toggled only on a press.
- **Short press (`24e1c0f`):** `press_start` and the last edge time read over the ST-Link while the button was used
  (memory reads about three times a second): holds of 2561 ms and 1974 ms left PB6 unchanged, a press of about 100 ms
  toggled it on the release. `press_start` came 30 ms after the press's last edge (edge at 147154 ms, `press_start`
  147184 ms).
- **Final code:** a short press toggles PB6 on the release, a 2 s press toggles the blue LED on the release, the press
  itself changes nothing, PB7 keeps blinking.
- **Video** of the board running it: https://photos.app.goo.gl/uAsFE9rx2HPPwmGD6 (Google Photos).

Build (Debug, `-O0 -g3`): 0 errors, 0 warnings with CubeIDE's flags and with `-Wextra`. `arm-none-eabi-size`: `text`
6308, `data` 12, `bss` 1580 bytes.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder (opened in place, not copied into the
  workspace).
- **Build:** Project → Build Project (the hammer).
- **Flash:** Run → Run As → STM32 C/C++ Application, with the ST-Link connected. Launch files (`*.launch`) hold
  absolute paths and are not committed, so the first Run creates one.
- **Change pins or the clock** in `00-homework-button-debounce-stm32.ioc` with CubeMX, then regenerate: everything in
  `main.c` outside the `USER CODE` blocks is rewritten. Open the `.ioc` from this folder: a CubeMX window still
  holding an older copy of the project generates into that copy's path.
