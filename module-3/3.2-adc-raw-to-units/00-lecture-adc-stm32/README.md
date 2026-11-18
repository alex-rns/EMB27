# 00 — Lecture: two ADC channels sampled by a timer and moved by DMA into a double buffer

[← Lesson 3.2](../)

The project the teacher built in class. TIM2 ticks 1000 times a second, and every tick starts one scan of ADC1: rank 1
reads a potentiometer on PA0, rank 2 an LDR divider on PA1. DMA2 copies each result into the next cell of a buffer
of 2 halves × 16 pairs and wraps around at its end (circular mode), with no CPU work. When a half is full (every
16 ms), HAL calls a callback that only notes which half is ready and counts it; `AdcDmaLab_Process()` in the main loop
then takes that half, splits the pairs into one array per sensor and stores the average and the millivolts of each.

The module `adc_dma_lab.c` / `adc_dma_lab.h` is the teacher's, as given. `main.c` only calls `AdcDmaLab_Start()` once
and `AdcDmaLab_Process()` in `while (1)`.

```
TIM2 update, 1 kHz ──► ADC1 scan: PA0, PA1 ──► DMA2 Stream 0 ──► dma_buffer[2][16] = {pot, light} pairs
                                                                    │ half 0 full: HT ─┐
                                                                    │ half 1 full: TC ─┤ callback: ready_half, count++
                                                                    ▼                  ▼
                                               main loop: AdcDmaLab_Process() takes the finished half
```

## Project

| Pin | CubeMX setting | Used for |
|---|---|---|
| PA0 | ADC1_IN0, rank 1 | the potentiometer's wiper |
| PA1 | ADC1_IN1, rank 2 | the LDR divider's midpoint |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |

**Clock:** the internal HSI 16 MHz, no PLL: SYSCLK, AHB, APB1 and APB2 all 16 MHz.
**ADC1:** PCLK2 / 2 = 8 MHz, 12 bits, right-aligned, scan of 2 channels at 144 cycles each: 144 + 12 = 156 ADC clocks
= 19.5 µs per conversion, 39 µs per pair. Continuous conversion off; the external trigger is TIM2 TRGO on the rising
edge; DMA Continuous Requests on; EOC at the end of the whole sequence.
**TIM2:** prescaler 159, period 99: 16 MHz / 160 / 100 = 1 kHz; TRGO = update.
**DMA2 Stream 0** (channel 0, ADC1's route): peripheral to memory, circular, half-word on both sides, memory
increment, no FIFO; its interrupt at priority 0. `MX_DMA_Init()` runs before `MX_ADC1_Init()`.
The ADC interrupt (overrun only) is enabled by `AdcDmaLab_Start()`, not in CubeMX.

CubeMX's target is the STM32F411**CC**Ux (256 KB flash, the linker script `STM32F411CCUX_FLASH.ld`); the Black Pill
carries the CE (512 KB). Both are STM32F411s with 128 KB of RAM; the CC's linker script just declares less flash, so
the program fits and runs.
STM32Cube FW_F4 V1.28.3, CubeMX 6.18.1, CubeIDE 2.2.0.

## Wiring

| Part | Connection |
|---|---|
| Potentiometer | its ends to the Black Pill's `3V3` and `G` (GND), the wiper to PA0 |
| LDR divider | the LDR and a fixed resistor in series between `3V3` and `G`, the midpoint to PA1 |
| SWD header | ST-Link V2: 3.3V, SWDIO, SWCLK, GND; the board runs from the ST-Link |

## How it works

- **`AdcDmaLab_Start()`** clears the counters and the buffers, sets TIM2's counter to 0, arms the ADC and its DMA
  (`HAL_ADC_Start_DMA`, length 64: the number of values, not bytes), enables the ADC interrupt for errors, and starts
  TIM2 last, so the first trigger finds the DMA ready.
- **The callbacks** (`HAL_ADC_ConvHalfCpltCallback`, `HAL_ADC_ConvCpltCallback`) set `ready_half` (0 or 1) and
  increment `completed_blocks`, nothing else: an interrupt must end quickly. `HAL_ADC_ErrorCallback` stores the error.
- **`AdcDmaLab_Process()`** returns at once while no new half has come. Otherwise it checks that the DMA is not
  writing into that half right now (from the DMA's count of values left), splits the 16 pairs into the two sensors'
  arrays, and stores the rounded average, the last reading and the millivolts (`average × 3300 / 4095`, rounded).
  It counts the halves it missed in `dropped_blocks`.
- **`AdcDmaLab_Stop()`** stops TIM2 first, then the ADC and its DMA.

## Notes

- **One array, interleaved.** The DMA doesn't know which sensor a value belongs to: it writes pot, light, pot,
  light… The order comes from the ADC ranks, and the struct `AdcDmaLab_Pair {potentiometer; light;}` lies in memory
  in the same order. Swap the ranks in CubeMX without swapping the fields, and the two sensors trade places.
- **The buffer length must be a multiple of the channel count.** With an odd number of values, every wrap of the
  circular buffer would shift the pairs by one.
- **The main loop has 16 ms per half.** If it takes longer, the DMA comes back around and overwrites the half before
  it is read: `AdcDmaLab_Process()` skips a half the DMA is in and counts it in `dropped_blocks`.
- **The timer and the DMA don't stop at a breakpoint.** A paused core misses halves; `dropped_blocks` was 1 after a
  debug session, most likely from such a pause.
- **An overrun stops the measurement.** If a conversion finishes before the DMA took the last one, everything after
  it is shifted. The ADC error callback records it, `AdcDmaLab_Process()` copies it to `error_code` and calls
  `AdcDmaLab_Stop()`; only another `AdcDmaLab_Start()` restarts it.
- **PA0 is the board's KEY button.** Pressing it pulls PA0 to GND through 330 Ω and drags the potentiometer's
  reading down.
- **The ADC's reference is the board's 3V3** (VREF+ is tied to VDDA on this package). The millivolts assume exactly
  3300 mV; the raw codes are a ratio of the input to the rail.
- **Zeros in Live Expressions** right after Debug: the core is paused at `main()` until Resume.

## Checked on the board

Values read in CubeIDE's Live Expressions while the program runs:

| Check | Result |
|---|---|
| The potentiometer, turned | 840 → 2116 → 4093 |
| The LDR | between about 1945 and 3244 |
| Halves processed | `processed_blocks` grows by about 60 a second (1000 / 16 = 62.5 expected) |
| Errors | `error_code` 0 |

The build: `.github/scripts/build-stm32.sh` (CubeIDE's GCC 14.3 with `-Wextra`) gives 0 warnings; `text` 13 980,
`data` 12, `bss` 2108 bytes.

## Working with the step

- **Open:** in CubeIDE, File → Open Projects from File System → this folder (opened in place, not copied into the
  workspace).
- **Build:** Project → Build Project (the hammer).
- **Flash:** Run → Run As → STM32 C/C++ Application, with the ST-Link connected. Launch files (`*.launch`) hold
  absolute paths and are not committed, so the first Run creates one.
- **Change pins or peripherals** in `00-lecture-adc-stm32.ioc` with CubeMX, then regenerate: everything in CubeMX's
  files outside the `USER CODE` blocks is rewritten.
