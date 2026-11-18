#include "adc_dma_lab.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

#define ADC_MAX_VALUE 4095U


volatile AdcDmaLab_Data adc_dma_lab __attribute__((aligned(4)));

typedef struct
{
  volatile uint32_t completed_blocks;
  volatile uint32_t ready_half;
  volatile uint32_t acquisition_error;
  uint32_t consumed_blocks;
  uint8_t running;
} AdcDmaLab_State;

static AdcDmaLab_State state;

/* Read only the half that DMA is not currently filling. */
static uint8_t HalfIsInactive(uint32_t half)
{
  uint32_t remaining = __HAL_DMA_GET_COUNTER(hadc1.DMA_Handle);
  uint32_t writing_half = (remaining > ADC_DMA_LAB_VALUES_PER_HALF) ? 0U : 1U;

  return (remaining != 0U) && (half != writing_half);
}

static void UpdateChannel(volatile AdcDmaLab_Channel *channel,
                          uint32_t half, uint32_t sum)
{
  uint32_t average = (sum + ADC_DMA_LAB_PAIRS_PER_HALF / 2U) /
                     ADC_DMA_LAB_PAIRS_PER_HALF;

  channel->raw = channel->samples[half][ADC_DMA_LAB_PAIRS_PER_HALF - 1U];
  channel->average = (uint16_t)average;
  channel->millivolts =
    (uint16_t)((average * ADC_DMA_LAB_VREF_MV + ADC_MAX_VALUE / 2U) /
               ADC_MAX_VALUE);
}

HAL_StatusTypeDef AdcDmaLab_Start(void)
{
  if (state.running != 0U)
  {
    return HAL_BUSY;
  }
  state = (AdcDmaLab_State){0};
  adc_dma_lab.processed_blocks = 0U;
  adc_dma_lab.dropped_blocks = 0U;
  adc_dma_lab.error_code = HAL_ADC_ERROR_NONE;
  for (uint32_t half = 0U; half < ADC_DMA_LAB_HALF_COUNT; ++half)
  {
    for (uint32_t pair = 0U; pair < ADC_DMA_LAB_PAIRS_PER_HALF; ++pair)
    {
      adc_dma_lab.dma_buffer[half][pair] = (AdcDmaLab_Pair){0};
      adc_dma_lab.light.samples[half][pair] = 0U;
      adc_dma_lab.potentiometer.samples[half][pair] = 0U;
    }
  }
  UpdateChannel(&adc_dma_lab.light, 0U, 0U);
  UpdateChannel(&adc_dma_lab.potentiometer, 0U, 0U);

  /* Do not generate an update event after arming the ADC: it is a trigger. */
  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

  /* DMA must be armed before starting the timer. Length counts halfwords. */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(void *)adc_dma_lab.dma_buffer,
                        ADC_DMA_LAB_BUFFER_SAMPLES) != HAL_OK)
  {
    (void)HAL_ADC_Stop_DMA(&hadc1);
    return HAL_ERROR;
  }

  /* HAL_ADC_Start_DMA enables the ADC overrun source; route it to HAL.
   * Regular samples use DMA HT/TC interrupts, not ADC EOC interrupts.
   */
  HAL_NVIC_SetPriority(ADC_IRQn, 1U, 0U);
  HAL_NVIC_ClearPendingIRQ(ADC_IRQn);
  state.running = 1U;
  HAL_NVIC_EnableIRQ(ADC_IRQn);

  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
  {
    (void)AdcDmaLab_Stop();
    return HAL_ERROR;
  }

  return HAL_OK;
}

uint8_t AdcDmaLab_Process(void)
{
  uint32_t sequence;
  uint32_t half;
  uint32_t pair;
  uint32_t light_sum = 0U;
  uint32_t potentiometer_sum = 0U;

  if (state.running == 0U)
  {
    return 0U;
  }

  if (state.acquisition_error != HAL_ADC_ERROR_NONE)
  {
    adc_dma_lab.error_code = state.acquisition_error;
    (void)AdcDmaLab_Stop();
    return 0U;
  }

  sequence = state.completed_blocks;
  if (sequence == state.consumed_blocks)
  {
    return 0U;
  }

  half = state.ready_half;
  if (HalfIsInactive(half) == 0U)
  {
    return 0U;
  }


  __DMB();
  /* Split the DMA pairs into separate sensor arrays and sum in one pass. */
  for (pair = 0U; pair < ADC_DMA_LAB_PAIRS_PER_HALF; ++pair)
  {
    AdcDmaLab_Pair sample = adc_dma_lab.dma_buffer[half][pair];

    adc_dma_lab.light.samples[half][pair] = sample.light;
    adc_dma_lab.potentiometer.samples[half][pair] = sample.potentiometer;
    light_sum += sample.light;
    potentiometer_sum += sample.potentiometer;
  }

  UpdateChannel(&adc_dma_lab.light, half, light_sum);
  UpdateChannel(&adc_dma_lab.potentiometer, half, potentiometer_sum);

  adc_dma_lab.dropped_blocks += sequence - state.consumed_blocks - 1U;
  state.consumed_blocks = sequence;
  ++adc_dma_lab.processed_blocks;
  return 1U;
}

HAL_StatusTypeDef AdcDmaLab_Stop(void)
{
  HAL_StatusTypeDef timer_status;
  HAL_StatusTypeDef adc_status;

  /* Stop the trigger first, then stop the ADC and its DMA stream. */
  timer_status = HAL_TIM_Base_Stop(&htim2);
  adc_status = HAL_ADC_Stop_DMA(&hadc1);
  HAL_NVIC_DisableIRQ(ADC_IRQn);
  HAL_NVIC_ClearPendingIRQ(ADC_IRQn);
  state.running = 0U;

  return ((timer_status == HAL_OK) && (adc_status == HAL_OK)) ?
         HAL_OK : HAL_ERROR;
}


void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  (void)hadc;
  state.ready_half = 0U;  /* First half ready; DMA is filling the second. */
  ++state.completed_blocks;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  (void)hadc;
  state.ready_half = 1U;  /* Second half ready; DMA is filling the first. */
  ++state.completed_blocks;
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  state.acquisition_error = HAL_ADC_GetError(hadc);
}


void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1);
}
