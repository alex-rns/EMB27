# 00 — Lecture: hardware PWM from TIM1, its duty set by a potentiometer through the ADC interrupt

[← Lesson 3.3](../)

The project built along with the lecture (STM32CubeIDE, HAL in C). TIM1 generates PWM in hardware on channel 1
(PA8) and on its complementary output CH1N (PA7, the inverted copy). PA8 drives a small DC motor through an NPN
2N2222, PA7 an LED. ADC1 reads a potentiometer on PA1 continuously; after each conversion its interrupt stores the
result, and the main loop turns it into the timer's compare value, so the knob sets the motor's duty (and the LED
gets the rest).

```
PA1 pot ──► ADC1 (continuous) ──► EOC interrupt: adc_value, adc_ready = 1
                                        │
main loop: adc_ready? ──► CCR1 = adc_value / 4 (0..1000) ──► TIM1 CH1  (PA8) ──► 2N2222 ──► motor
                                                           └► TIM1 CH1N (PA7, inverted) ──► LED
```

## Project

| Pin | CubeMX setting | Used for |
|---|---|---|
| PA8 | TIM1_CH1, PWM Generation CH1 CH1N | the transistor's base, through a resistor |
| PA7 | TIM1_CH1N | the LED, through a resistor |
| PA1 | ADC1_IN1 | the potentiometer's middle pin |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |

**Clock:** the internal HSI 16 MHz, no PLL: SYSCLK, APB1 and APB2 all 16 MHz.
**TIM1:** prescaler 1600 - 1, period 1000 - 1: 16 MHz / 1600 / 1000 = 10 Hz, slow enough to see the PWM by eye.
The duty is CCR1 / 1000; in PWM mode 1 the output is high while the counter is below CCR1 (RM0383).
**ADC1:** PCLK2 / 2 = 8 MHz, 12 bits, one channel (IN1), continuous conversion started by software
(`HAL_ADC_Start_IT`), 480 + 12 = 492 ADC clocks ≈ 61.5 µs a conversion, EOC after each conversion, the ADC1 global
interrupt on (CubeMX NVIC).
CubeMX's target is the STM32F411CEUx. STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

## Code (in `main.c`'s `USER CODE` blocks)

- `USER CODE 2`: `HAL_TIM_PWM_Start` (CH1) and `HAL_TIMEx_PWMN_Start` (CH1N), a starting duty of 25 %
  (`__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 250)`), then `HAL_ADC_Start_IT(&hadc1)`.
- `HAL_ADC_ConvCpltCallback` (`USER CODE 4`): reads ADC1's data register into `volatile adc_value` and sets
  `adc_ready`.
- The main loop: when `adc_ready` is set, `adc_corrected = adc_value > 4000 ? 1000 : adc_value / 4`, written to CCR1,
  then `adc_ready` is cleared. The clamp keeps 4001..4095 from going past 1000.

## Wiring

| Part | Connection |
|---|---|
| Potentiometer | its ends to `3V3` and `G`, the middle pin to PA1 |
| Motor driver | PA8 → resistor → 2N2222 base; emitter to `G`; collector to the motor's minus; the motor's plus to its supply; a flyback diode across the motor, its stripe (cathode) to the motor's plus |
| LED | PA7 → resistor → LED → `G` |
| SWD header | ST-Link V2 |

The kit's 2N2222 is marked P2N2222A: with the flat side facing, the legs are collector, base, emitter (onsemi
TO-92 style 17; the PN2222A without the "P" is the other way round).

## Notes

- **No ADC interrupt in CubeMX, no callback.** Without the NVIC tick for ADC1 there is no `ADC_IRQHandler`; the linker
  (`--gc-sections`) then drops the callback and `adc_value` with it, and the debugger shows "Failed to evaluate
  expression". Enabling `ADC_IRQn` by hand (`HAL_NVIC_EnableIRQ`) without the handler sends the first interrupt to
  `Default_Handler`, an endless loop. The NVIC tick makes the handler and enables the interrupt in `HAL_ADC_MspInit`.
- **The scale must match the period.** With a period of 1000, CCR 1000 is 100 %; dividing the ADC value by 40 (for a
  period of 100) would leave the duty at 0–10 %.
- **The flyback diode's direction matters.** Backwards, it shorts the motor whenever the transistor is on.
- **The 5V pin has no power from the ST-Link alone:** on the Black Pill it is fed from the USB-C port.
- **A small motor needs some duty to start:** at low duty it doesn't turn.
- **The frequency is from the HSI:** the logic analyzer measured 9.81–9.82 Hz instead of 10 Hz.

## Checked on the board

Logic analyzer (1 MS/s) on PA8 and PA7:

| Setting | PA8 (CH1) | PA7 (CH1N) |
|---|---|---|
| CCR1 = 250 | 9.81 Hz, 25 % high | 75 % high, complementary |
| the knob a little above the middle | period 101.8 ms, 57–58 % high | 42–43 % high |

The motor's speed and the LED's brightness follow the knob in opposite directions. The build:
`.github/scripts/build-stm32.sh` (CubeIDE's GCC 14.3 with `-Wextra`) gives 0 warnings.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder.
- **Build / flash:** the hammer, then Run → Run As → STM32 C/C++ Application with the ST-Link connected.
- **Change pins or peripherals** in `00-lecture-pwm-stm32.ioc` with CubeMX, then regenerate: everything outside the
  `USER CODE` blocks is rewritten.
