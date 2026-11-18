#ifndef ADC_DMA_LAB_H
#define ADC_DMA_LAB_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_DMA_LAB_CHANNEL_COUNT    2U
#define ADC_DMA_LAB_HALF_COUNT       2U
#define ADC_DMA_LAB_PAIRS_PER_HALF  16U
#define ADC_DMA_LAB_VALUES_PER_HALF (ADC_DMA_LAB_PAIRS_PER_HALF * ADC_DMA_LAB_CHANNEL_COUNT)
#define ADC_DMA_LAB_BUFFER_SAMPLES  (ADC_DMA_LAB_HALF_COUNT * ADC_DMA_LAB_VALUES_PER_HALF)
#define ADC_DMA_LAB_VREF_MV       3300U

typedef struct
{
  uint16_t potentiometer;  /* PA0, ADC rank 1 */
  uint16_t light;          /* PA1, ADC rank 2 */
} AdcDmaLab_Pair;

typedef struct
{
  /* A separate array for this sensor: 2 halves x 16 samples. */
  uint16_t samples[ADC_DMA_LAB_HALF_COUNT][ADC_DMA_LAB_PAIRS_PER_HALF];
  uint16_t raw;
  uint16_t average;
  uint16_t millivolts;
} AdcDmaLab_Channel;

typedef struct
{
  /* DMA writes 64 interleaved values: light, potentiometer, light, ... */
  AdcDmaLab_Pair dma_buffer[ADC_DMA_LAB_HALF_COUNT][ADC_DMA_LAB_PAIRS_PER_HALF];
  AdcDmaLab_Channel light;
  AdcDmaLab_Channel potentiometer;
  uint32_t processed_blocks;
  uint32_t dropped_blocks;
  uint32_t error_code;
} AdcDmaLab_Data;

/* Process updates sensor arrays in place, including on a rejected copy.
 * Measurement results and block counters are committed after validation.
 */
extern volatile AdcDmaLab_Data adc_dma_lab;


/* Call after MX initialization; ADC DMAContinuousRequests must be ENABLE. */
HAL_StatusTypeDef AdcDmaLab_Start(void);
uint8_t AdcDmaLab_Process(void);
HAL_StatusTypeDef AdcDmaLab_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_DMA_LAB_H */
