# 00 — Lecture: an SG90 servo on TIM4 PWM, its angle set by a potentiometer, all on registers

[← Lesson 3.5](../)

The lecture's code (STM32CubeIDE, C on CMSIS registers, no HAL calls). CubeMX made a bare project (the clock only);
the code replaces `main.c` and `main.h` and sets every peripheral itself: the clock, SysTick, TIM4 and ADC1. TIM4
counts 1 µs ticks with a period of 20 ms (50 Hz) and puts the pulse out on PB6 (TIM4_CH1). The main loop averages 8
ADC readings of a potentiometer on PA0, maps 0..4095 to a 500..2500 µs pulse, writes it to CCR1 and waits one period.

```
PA0 pot ──► ADC1 (single conversion, polled, 8 averaged) ──► 500 + adc × 2000 / 4095 µs
                                                                    │
                                     TIM4: 1 MHz tick, ARR 19999 ──► CCR1 ──► PB6 (TIM4_CH1, AF2) ──► servo signal
```

## Project

| Pin | Set by | Used for |
|---|---|---|
| PB6 | the code: AF2 (TIM4_CH1), push-pull, low speed, no pull | the servo's signal wire |
| PA0 | the code: analog mode, no pull | the potentiometer's middle pin (ADC1_IN0) |

**CubeMX:** nothing but the clock (HSI 16 MHz, no PLL). STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

## Code (`Core/Src/main.c`, `Core/Inc/main.h`)

- `Clock_Init()`: HSI on and selected as SYSCLK; AHB, APB1, APB2 undivided, so the core and TIM4 run at 16 MHz;
  `SystemCoreClock = HSI_VALUE`.
- `SysTick_Init()` / `Delay_ms()`: SysTick reloads every 1 ms without an interrupt (TICKINT = 0); `Delay_ms(n)`
  counts n COUNTFLAG wraps.
- `Servo_Init()`: the GPIOB and TIM4 clocks, a TIM4 reset, PB6 to AF2; TIM4 with PSC = 16 MHz / 1 MHz - 1 = 15
  (a 1 µs tick), ARR = 1 MHz / 50 Hz - 1 = 19999 (20 ms), CCR1 = 1500 µs (the centre), PWM mode 1 with buffered ARR
  and CCR1, an update event to load them, then the CH1 output and the counter on.
- `ADC1_Init()` / `ADC1_Read()`: the GPIOA and ADC1 clocks, PA0 analog, ADCCLK = PCLK2 / 4 = 4 MHz, 12 bits,
  channel 0 alone with 480-cycle sampling; a read starts a conversion (SWSTART) and waits for EOC.
- `main()`: every 20 ms, 8 readings averaged, `pulse = 500 + adc × 2000 / 4095` (rounded) into `TIM4->CCR1`.
- `main.h`: the timing and pulse constants (`SERVO_MIN_US` 500, `SERVO_CENTER_US` 1500, `SERVO_MAX_US` 2500).

**Changed from the teacher's code:** the comments were translated, and two things the bare project needs:
- `main.h` includes `stm32f4xx_hal.h` (which pulls in `stm32f411xe.h`) instead of `stm32f411xe.h` alone: CubeMX's
  `stm32f4xx_it.c` and `stm32f4xx_hal_msp.c` get `HAL_IncTick()` and `__HAL_RCC_..._CLK_ENABLE()` through it;
- three TIM4 lines use CMSIS bit names instead of HAL ones, with the same values: CubeMX copies the HAL timer driver
  only when the `.ioc` uses a timer, so `TIM_AUTORELOAD_PRELOAD_ENABLE`, `TIM_OCMODE_PWM1`, `TIM_OUTPUTSTATE_ENABLE`
  and `TIM_OCPOLARITY_HIGH` don't exist here. They are `TIM_CR1_ARPE`, `TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1`,
  `TIM_CCER_CC1E` and 0.

## Wiring

| Part | Connection |
|---|---|
| SG90 servo | orange (signal) to PB6, red to +5 V, brown to `G` |
| Servo supply | 5 V from the breadboard power module (MB-102), its ground joined to the Black Pill's `G` |
| Potentiometer | its ends to `3V3` and `G`, the middle pin to PA0 |
| SWD header | ST-Link V2 |

## Checked on the board

- **Registers** (read over the ST-Link with the potentiometer still): TIM4 CR1 CEN = 1 and ARPE = 1, PSC = 15,
  ARR = 19999, CCER CC1E = 1; GPIOB MODER6 = 10 and AFRL6 = 2; ADC1 DR 1334–1336 and CCR1 1152–1153, as the formula
  gives (500 + 1335 × 2000 / 4095 ≈ 1152).
- **Logic analyzer** on PB6 (4 MS/s, 0.5 s): a period of 20.33 ms (49.2 Hz) every time, the pulse 2438–2461 µs
  with the knob near one end.
- The servo follows the knob.

## Notes

- **The timing comes from the HSI.** The period measured 20.33 ms instead of 20.00 ms: the internal RC oscillator
  runs about 1.6 % slow on this board (3.3's TIM1 measured 1.9 % slow too). DS10314 Table 39 gives ±1 % at 25 °C
  for unsoldered parts and -4..+4 % over -10..85 °C. Every time stretches by the same factor, the pulse too; the
  servo doesn't mind a period off by a few percent.
- **500..2500 µs is wider than the SG90's nominal 1..2 ms.** Many SG90s turn further with it, but the ends differ
  per unit: a pulse past the mechanical stop makes the servo push against it, buzz and draw current. Calibrate
  `SERVO_MIN_US` and `SERVO_MAX_US` for the servo at hand.
- **The servo needs its own 5 V.** It draws about 100–250 mA moving and more when stalled; the Black Pill's 3V3
  regulator is not for that, and its `5V` pin is fed only from USB-C. The grounds must be joined, or PB6's pulse has
  no reference.
- **PA0 also carries the KEY button** (PA0 → 330 Ω → button → `G`): pressing it pulls the potentiometer's voltage
  down and the servo moves.
- **Don't regenerate this project in CubeMX.** `main.c` has no `USER CODE` blocks, so Generate Code would replace the
  whole file with CubeMX's default.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder.
- **Build / flash:** the hammer, then Run → Run As → STM32 C/C++ Application with the ST-Link connected.
