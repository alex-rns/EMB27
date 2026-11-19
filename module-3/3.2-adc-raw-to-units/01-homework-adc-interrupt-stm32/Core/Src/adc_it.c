#include "adc_it.h"

/* CubeMX's handles, defined in main.c. */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

#define TIM2_CLOCK_HZ   16000000U  /* TIM2's clock: APB1 = 16 MHz here (HSI, no PLL) */
#define NORMAL_RATE_HZ  1000U      /* the trigger rate for normal work */
#define SWEEP_STEP_MS   100U       /* about how long the search tries each rate (counted in conversions) */
#define SWEEP_MIN_TICKS 2U         /* the shortest period TIM2 can make: ARR = 1 */

/* Written by the ADC interrupt, read outside it (by the search, or in the debugger): volatile, so every read
 * goes to RAM. */
static volatile uint16_t s_adc_raw;        /* the last conversion, 0..4095; read in the debugger */
static volatile uint32_t s_adc_irq_count;  /* conversions taken so far; read in the debugger: 1000 a second at 1 kHz */
static volatile uint32_t s_adc_ovr_count;  /* overruns: readings lost; read by the search */
/* The search's results, read in the debugger. */
static volatile uint32_t s_sweep_ok_hz;      /* the fastest trigger rate with no overrun */
static volatile uint32_t s_sweep_fail_hz;    /* the first rate that lost a reading; 0: none */
static volatile uint32_t s_sweep_step_left;  /* conversions left in the search's step; 0: no step */

/* Load a new trigger period into a stopped TIM2: prescaler 0 (one tick = 1/16 us), the period in ticks. The
 * update event (UG) applies both at once and restarts the count from 0; as TRGO it also triggers one conversion. */
static void trigger_load_period(uint32_t ticks)
{
  __HAL_TIM_SET_PRESCALER(&htim2, 0U);
  __HAL_TIM_SET_AUTORELOAD(&htim2, ticks - 1U);  /* the timer counts 0..ARR: ARR + 1 ticks */
  htim2.Instance->EGR = TIM_EGR_UG;
}

HAL_StatusTypeDef adc_it_start(void)
{
  HAL_StatusTypeDef status;

  /* TIM2 counts at the full 16 MHz (CubeMX set prescaler 159), so the rate can be set finely. */
  trigger_load_period(TIM2_CLOCK_HZ / NORMAL_RATE_HZ);
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
 * (with HAL_Delay and the SysTick) would never run again. */
void adc_it_find_max_rate(void)
{
  uint32_t ticks = TIM2_CLOCK_HZ / NORMAL_RATE_HZ;

  while (ticks >= SWEEP_MIN_TICKS)
  {
    uint32_t hz = TIM2_CLOCK_HZ / ticks;

    __HAL_TIM_DISABLE(&htim2);
    trigger_load_period(ticks);
    s_adc_ovr_count = 0U;  /* no overrun can come while TIM2 is stopped */
    s_sweep_step_left = hz * SWEEP_STEP_MS / 1000U;
    __HAL_TIM_ENABLE(&htim2);
    while ((htim2.Instance->CR1 & TIM_CR1_CEN) != 0U)
    {
      /* the ADC interrupt stops TIM2: after the step's conversions, or at an overrun */
    }
    s_sweep_step_left = 0U;

    if (s_adc_ovr_count != 0U)
    {
      s_sweep_fail_hz = hz;
      break;
    }
    s_sweep_ok_hz = hz;
    ticks -= (ticks / 32U != 0U) ? (ticks / 32U) : 1U;  /* about 3 % faster each step */
  }

  __HAL_TIM_DISABLE(&htim2);
  trigger_load_period(TIM2_CLOCK_HZ / NORMAL_RATE_HZ);
  __HAL_TIM_ENABLE(&htim2);
}

/* HAL calls this from ADC_IRQHandler after every conversion (EOC). An interrupt must end quickly: take the
 * number and count it, nothing else. */
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
  /* Stop the trigger: readings are being lost, and an interrupt that never ends would starve main. */
  __HAL_TIM_DISABLE(&htim2);
}
