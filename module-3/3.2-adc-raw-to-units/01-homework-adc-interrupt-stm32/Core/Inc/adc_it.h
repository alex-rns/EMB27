#ifndef ADC_IT_H
#define ADC_IT_H

#include "stm32f4xx_hal.h"

/* ADC1 (PA0) in interrupt mode, no DMA, one conversion per TIM2 trigger. */

/* Set TIM2 to 1 kHz, arm the ADC, start TIM2. HAL_OK, or HAL's error status. */
HAL_StatusTypeDef adc_it_start(void);

/* The bonus: raise the trigger rate until the first overrun, then go back to 1 kHz. Blocks about 14 s with
 * 3-cycle sampling; about 2 min with 144 cycles, where the ADC tops out at about 51 000 conversions a second. */
void adc_it_find_max_rate(void);

#endif /* ADC_IT_H */
