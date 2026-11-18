# 01 — Homework (extra): the ADC in interrupt mode, triggered by a timer, and the fastest trigger before an overrun

[← Lesson 3.2](../)

The teacher's extra homework for lesson 3.2, on the WeAct Black Pill. ADC1 reads a potentiometer on PA0, one
conversion per TIM2 trigger, with no DMA: after each conversion the ADC's interrupt calls a callback that takes the
result from the ADC's data register itself. The bonus: the program raises the trigger rate step by step until the
ADC reports its first overrun.

The step starts from the lecture project ([00-lecture-adc-stm32](../00-lecture-adc-stm32/), copied unchanged), so
the git history of this folder goes: CubeMX without DMA, the interrupt and its callback, the overrun counter, TIM2's
period set from code, the search, the shorter sampling time, and the code moved into the `adc_it` module. `main.c`
only calls `adc_it_start()` and `adc_it_find_max_rate()`, as the lecture's called `AdcDmaLab_Start()`.

## Assignment

Extra homework 3.2 (the teacher, in Slack):

- Set up the ADC in interrupt mode, without DMA, triggered by a timer.
- Take the data out of the interrupt by hand.

Bonus points for finding the highest ADC trigger rate before an overrun error.

## How each point is done

| Point | Where | How |
|---|---|---|
| Interrupt mode, no DMA | `.ioc`, `adc_it.c` | the DMA request removed in CubeMX, the ADC1 global interrupt on; `HAL_ADC_Start_IT` enables the end-of-conversion (EOC) and overrun (OVR) interrupts |
| Triggered by a timer | `.ioc`, `adc_it.c` | external trigger TIM2 TRGO (update event), rising edge; TIM2 at 1 kHz |
| The data taken by hand | `adc_it.c` | `HAL_ADC_ConvCpltCallback` reads the data register with `HAL_ADC_GetValue` into `s_adc_raw` and counts it in `s_adc_irq_count` |
| Bonus: the fastest trigger before an overrun | `adc_it.c` | `adc_it_find_max_rate()` runs once after reset; `HAL_ADC_ErrorCallback` counts overruns. Result below |

```
TIM2 TRGO ──► ADC1 converts PA0 ──► EOC interrupt: ADC_IRQHandler → HAL_ADC_IRQHandler
                                       ├─ a result:  HAL_ADC_ConvCpltCallback → s_adc_raw, s_adc_irq_count++
                                       └─ overrun:   HAL_ADC_ErrorCallback   → s_adc_ovr_count++, TIM2 stopped
```

## The bonus: the fastest trigger before an overrun

**How it is searched.** `adc_it_find_max_rate()` starts TIM2 at 1 kHz and shortens its period by about 3 % a step
(prescaler 0, so the period is set in 1/16 µs ticks). Each rate runs for about 100 ms worth of triggers, counted in
conversions, and the step ends when the ADC interrupt stops TIM2. The search stops at the first overrun:
`HAL_ADC_ErrorCallback` counts it and stops TIM2 too. Then TIM2 goes back to 1 kHz. The results are read in the
debugger: `s_sweep_ok_hz` (the fastest rate with no overrun) and `s_sweep_fail_hz` (the first that lost a reading).

The step's end is counted by the interrupt, not timed by `main` with `HAL_Delay`, because the first version (with
`HAL_Delay`, at 3 cycles) stopped at 70 kHz: from there on the interrupt took all the CPU time, and `main` and the SysTick, which
has the lowest priority, never ran again.

**Results** (HSI 16 MHz, ADC clock 8 MHz, HAL, CubeIDE's Debug build at `-O0`):

| Sampling time | Result |
|---|---|
| 144 cycles: 144 + 12 = 156 ADC clocks, 19.5 µs a conversion | **no overrun at all**, up to an 8 MHz trigger (the fastest TIM2 makes, period 2 ticks). The ADC tops out at 51 257 conversions a second (1 / 19.5 µs); a trigger that comes during a conversion is ignored, without an error. At 53.7 kHz exactly every second trigger was converted |
| 3 cycles: 3 + 12 = 15 ADC clocks, 1.9 µs a conversion | **no overrun at 88 888 Hz** (period 11.25 µs), **the first at 91 428 Hz** (period 10.94 µs); the same in three runs |

So the answer to the bonus: about **89–91 kHz**, and only once the ADC converts faster than the interrupt takes its
results; then the interrupt is the limit. With 144 cycles the ADC is the limit, and it never overruns.

Seen along the way: from about 70 kHz (period 14.2 µs) the interrupt takes all the CPU time without losing a
reading. With TIM2 fixed at 70 485 Hz or at 88 888 Hz, conversions kept coming with no overrun while `uwTick` (HAL's
millisecond tick) stayed 0 from reset; at 68 376 Hz `HAL_Delay` still returned. Why the CPU is full from about 70 kHz
but readings are lost only from about 91 kHz is not measured here: a pin toggled at the start and the end of the
interrupt, seen on the logic analyzer, would show how long the interrupt takes and when it reads the data register.

## Project

| Pin | CubeMX setting | Used for |
|---|---|---|
| PA0 | ADC1_IN0 | the potentiometer's wiper |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |

**Clock:** the internal HSI 16 MHz, no PLL: SYSCLK, AHB, APB1 and APB2 all 16 MHz.
**ADC1:** PCLK2 / 2 = 8 MHz, 12 bits, right-aligned, one channel (no scan), sampled for 3 cycles; no continuous
conversion; the external trigger TIM2 TRGO on the rising edge; DMA off; EOC after each conversion; the ADC1
interrupt at priority 0 (the SysTick at 15, the lowest).
**TIM2:** CubeMX sets prescaler 159 and period 99 (1 kHz), TRGO = update; `adc_it_start()` reloads it with prescaler 0
and a period of 16 000 ticks, still 1 kHz.
CubeMX's target is the STM32F411CCUx, as in the lecture project; the Black Pill carries the CE (see the lecture
step's README). STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

## Wiring

| Part | Connection |
|---|---|
| Potentiometer | its ends to the Black Pill's `3V3` and `G` (GND), the wiper to PA0 |
| SWD header | ST-Link V2: 3.3V, SWDIO, SWCLK, GND; the board runs from the ST-Link |

## Notes

- **Without DMA, an overrun is reported only with EOC after each conversion** (CubeMX "End Of Conversion Selection:
  EOC flag at the end of single channel conversion", the EOCS bit; RM0383 11.8.2). With EOC at the end of the
  sequence and no DMA, the ADC never sets OVR, and the search would find no limit.
- **A trigger during a conversion is ignored, not an error.** Fewer conversions than triggers is the only sign.
- **An interrupt that takes all the CPU time gives no error either.** `main` stops, and so does the HAL tick, so
  `HAL_Delay` never returns. The error callback stops TIM2, so after an overrun the program gets the CPU back.
- **3 cycles are too short for full accuracy through the potentiometer.** The datasheet's formula for an error below
  ¼ LSB allows at most about 2 kΩ of source resistance at 3 cycles and 8 MHz (with the typical 4 pF sample
  capacitor; none with the maximum 7 pF), and a 10 kΩ potentiometer's wiper has up to 2.5 kΩ. The setting is for the
  bonus; 144 cycles (allowing the datasheet's 50 kΩ maximum) were used before it.
- **The search blocks `main` for about 14 s after every reset**, then the ADC runs at 1 kHz.
- **The results depend on the build.** At `-O2`, or with a faster clock (up to 100 MHz with the PLL), the interrupt would be
  shorter, so the overrun should come later (not measured).

## Checked on the board

Values read from RAM over the ST-Link while the program runs:

| Check | Result |
|---|---|
| 1 kHz, 144 cycles | `s_adc_irq_count` equal to `uwTick` (1000 conversions a second), `s_adc_ovr_count` 0 |
| The potentiometer, 144 cycles | 0–5 at one end, 4090–4095 at the other |
| TIM2 after `adc_it_start()` | `PSC` 0, `ARR` 15 999 |
| The search, 144 cycles | no overrun up to 8 MHz; 51 257 conversions a second at the end |
| The search, 3 cycles | `s_sweep_ok_hz` 88 888, `s_sweep_fail_hz` 91 428, three runs (the last after the refactor) |
| After the search | back to 1 kHz: `s_adc_irq_count` grows with `uwTick` |

The build: `.github/scripts/build-stm32.sh` (CubeIDE's GCC 14.3 with `-Wextra`) gives 0 warnings; `text` 10 656,
`data` 12, `bss` 1740 bytes.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder (opened in place, not copied into the
  workspace). It can be open next to the lecture project: the names differ.
- **Build:** Project → Build Project (the hammer). After a pull that adds `.c` files, refresh the project (F5) first.
- **Flash:** Run → Run As → STM32 C/C++ Application, with the ST-Link connected. Launch files (`*.launch`) hold
  absolute paths and are not committed, so the first Run creates one.
- **Change pins or peripherals** in `01-homework-adc-interrupt-stm32.ioc` with CubeMX, then regenerate: everything in
  CubeMX's files outside the `USER CODE` blocks is rewritten.
