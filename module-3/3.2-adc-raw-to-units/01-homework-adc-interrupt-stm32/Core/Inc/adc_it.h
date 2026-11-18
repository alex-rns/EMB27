#ifndef ADC_IT_H
#define ADC_IT_H

#include "stm32f4xx_hal.h"

/* ADC1 (PA0) in interrupt mode, no DMA, one conversion per TIM2 trigger. */

/* Set TIM2 to 1 kHz, arm the ADC, start TIM2. HAL_OK, or HAL's error status. */
HAL_StatusTypeDef adc_it_start(void);

/* The bonus: raise the trigger rate until the first overrun (blocks about 12 s), then go back to 1 kHz. */
void adc_it_find_max_rate(void);

#endif /* ADC_IT_H */
