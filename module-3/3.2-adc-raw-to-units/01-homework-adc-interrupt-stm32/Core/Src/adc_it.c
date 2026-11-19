#include "adc_it.h"

/* CubeMX's handles, defined in main.c. */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

#define NORMAL_RATE_HZ  1000U  /* the trigger rate for normal work */
#define SWEEP_STEP_MS   100U   /* about how long the search tries each rate (counted in conversions) */
#define SWEEP_MIN_TICKS 2U     /* the shortest period TIM2 can make: ARR = 1 */

/* Written by the ADC interrupt, read outside it (by the search, or in the debugger): volatile, so every read
 * goes to RAM. */
static volatile uint16_t s_adc_raw;        /* the last conversion, 0..4095; read in the debugger */
static volatile uint32_t s_adc_irq_count;  /* conversions taken so far; read in the debugger: 1000 a second at 1 kHz */
static volatile uint32_t s_adc_ovr_count;  /* overruns: readings lost; read by the search */
/* The search's results, read in the debugger. */
static volatile uint32_t s_sweep_ok_hz;      /* the fastest trigger rate with no overrun */
static volatile uint32_t s_sweep_fail_hz;    /* the first rate that gave an overrun; 0: none */
static volatile uint32_t s_sweep_step_left;  /* conversions left in the search's step; 0: no step */

/* TIM2's clock, read from the clock tree, so the rates stay right if the clock changes in CubeMX: PCLK1 when the
 * APB1 prescaler is 1, twice PCLK1 otherwise (RM0383 6.2, with TIMPRE at its reset value 0). 16 MHz here. */
static uint32_t tim2_clock_hz(void)
{
  uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();

  return ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1) ? pclk1 : 2U * pclk1;
}

/* Load a new trigger period into a stopped TIM2: prescaler 0 (one tick = one TIM2 clock cycle, 1/16 us at
 * 16 MHz), the period in ticks. The update event (UG) applies both at once and restarts the count from 0; as TRGO
 * it also triggers one conversion. */
static void trigger_load_period(uint32_t ticks)
{
  __HAL_TIM_SET_PRESCALER(&htim2, 0U);
  __HAL_TIM_SET_AUTORELOAD(&htim2, ticks - 1U);  /* the timer counts 0..ARR: ARR + 1 ticks */
  htim2.Instance->EGR = TIM_EGR_UG;
}

HAL_StatusTypeDef adc_it_start(void)
{
  HAL_StatusTypeDef status;

  /* TIM2 counts every clock cycle (CubeMX set prescaler 159), so the rate can be set finely. The ADC is still
   * off, so the conversion UG triggers here is ignored. */
  trigger_load_period(tim2_clock_hz() / NORMAL_RATE_HZ);
  /* Arm the ADC first: it waits for TIM2's trigger and interrupts after each conversion. */
  status = HAL_ADC_Start_IT(&hadc1);
  if (status != HAL_OK)
  {
    return status;
  }
  /* Start the trigger last, so its first tick finds the ADC ready. */
  return HAL_TIM_Base_Start(&htim2);
}

/* Start at NORMAL_RATE_HZ and shorten TIM2's period by about 3 % a step; run each rate for about SWEEP_STEP_MS
 * and stop at the first overrun. Then go back to NORMAL_RATE_HZ. Blocks main while it runs.
 * The ADC interrupt ends each step, not main: at the fastest rates the interrupt takes all the CPU time, and main
 * (with HAL_Delay and the SysTick) would never run again.
 * TIM2 is started and stopped with the raw CEN macros, so htim2.State stays BUSY: nothing calls
 * HAL_TIM_Base_Start() afterwards. */
void adc_it_find_max_rate(void)
{
  uint32_t clock_hz = tim2_clock_hz();
  uint32_t ticks = clock_hz / NORMAL_RATE_HZ;  /* the trigger period in TIM2 clock cycles: 16 000 = 1 kHz */

  while (ticks >= SWEEP_MIN_TICKS)
  {
    uint32_t hz = clock_hz / ticks;  /* the rate this step tries */

    /* Set the step up with TIM2 stopped: the counters first, then the period. Its update event (UG) triggers one
     * conversion, which counts as the step's first. */
    __HAL_TIM_DISABLE(&htim2);
    s_adc_ovr_count = 0U;
    s_sweep_step_left = hz * SWEEP_STEP_MS / 1000U;  /* SWEEP_STEP_MS worth of conversions at this rate */
    trigger_load_period(ticks);

    /* Run the step and wait for its end. */
    __HAL_TIM_ENABLE(&htim2);
    while ((htim2.Instance->CR1 & TIM_CR1_CEN) != 0U)
    {
      /* the ADC interrupt stops TIM2: after the step's conversions, or at an overrun */
    }
    s_sweep_step_left = 0U;  /* after an overrun some may be left: no step is running now */

    /* Read the step's result. */
    if (s_adc_ovr_count != 0U)
    {
      s_sweep_fail_hz = hz;  /* a reading was lost: this rate is too fast, the search ends */
      break;
    }
    s_sweep_ok_hz = hz;  /* no overrun at this rate */
    /* The next period is 1/32 shorter, about 3 % faster. Below 32 ticks 1/32 is 0, so take at least 1 tick off,
     * or the loop would never end. */
    ticks -= (ticks / 32U != 0U) ? (ticks / 32U) : 1U;
  }

  /* Back to normal work at 1 kHz. */
  __HAL_TIM_DISABLE(&htim2);
  trigger_load_period(clock_hz / NORMAL_RATE_HZ);
  __HAL_TIM_ENABLE(&htim2);
}

/* HAL calls this from ADC_IRQHandler after every conversion (EOC). An interrupt must end quickly: take the
 * number and count it, nothing else. HAL clears the EOC flag only after this returns, so a conversion that ends
 * in between would lose its interrupt without an overrun; at 1 kHz there is no such conversion. */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  s_adc_raw = (uint16_t)HAL_ADC_GetValue(hadc);  /* reads the ADC data register (DR), 12 bits */
  s_adc_irq_count++;
  if ((s_sweep_step_left != 0U) && (--s_sweep_step_left == 0U))
  {
    __HAL_TIM_DISABLE(&htim2);  /* the search's step is over */
  }
}

/* HAL calls this from ADC_IRQHandler on an overrun (OVR): a conversion finished before the last one was read,
 * so that reading is lost. Without DMA it is the only error HAL reports here. HAL clears the flag itself, and the
 * next trigger converts again. */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  (void)hadc;
  s_adc_ovr_count++;
  /* In a search step, stop the trigger: readings are being lost, and an interrupt that never ends would starve
   * main. In normal work keep going: an overrun also comes after a breakpoint, because TIM2 and the ADC keep
   * running while the core is halted, and stopping TIM2 then would end the sampling until a reset. */
  if (s_sweep_step_left != 0U)
  {
    __HAL_TIM_DISABLE(&htim2);
  }
}
