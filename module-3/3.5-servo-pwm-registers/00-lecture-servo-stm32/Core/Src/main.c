/*
 * Lesson 3.5: STM32F411CEU6, a servo driven from the ADC, on registers.
 * PB6 = TIM4_CH1 (AF2), PA0 = ADC1_IN0.
 * Addresses and masks: stm32f411xe.h (CMSIS); mode names: the HAL headers.
 */
#include "main.h"
#include "stm32f4xx_hal.h" /* HAL constants only. */

volatile uint16_t adc_raw;
volatile uint16_t adc_filtered;
volatile uint16_t servo_pulse_us = SERVO_CENTER_US;

static void Clock_Init(void);
static void SysTick_Init(void);
static void Delay_ms(uint32_t milliseconds);
static void Servo_Init(void);
static void ADC1_Init(void);
static uint16_t ADC1_Read(void);

int main(void)
{
  Clock_Init();
  SysTick_Init();
  Servo_Init();
  ADC1_Init();

  while (1)
  {
    /* The average of the ADC readings. */
    uint32_t adc_sum = 0U;
    for (uint32_t i = 0U; i < ADC_AVERAGE_SAMPLES; ++i)
    {
      adc_raw = ADC1_Read();
      adc_sum += adc_raw;
    }
    adc_filtered = (uint16_t)(adc_sum / ADC_AVERAGE_SAMPLES);
    /* ADC 0..4095 -> 500..2500 us; the end positions are calibrated for the servo. */
    servo_pulse_us = SERVO_MIN_US + ((uint32_t)adc_filtered * (SERVO_MAX_US - SERVO_MIN_US) + ADC_MAX_VALUE / 2U) / ADC_MAX_VALUE;
    TIM4->CCR1 = servo_pulse_us;                   /* TIM4: pulse width */
    Delay_ms(SERVO_UPDATE_MS);
  }
}

static void Clock_Init(void)
{
  RCC->CR |= RCC_CR_HSION;                        /* RCC: turn on HSI */
  while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {}      /* RCC: wait for HSI ready */
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_SYSCLKSOURCE_HSI; /* RCC: select HSI */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {} /* RCC: confirm the clock source */
  /* AHB /1, APB1 /1, APB2 /1: the core and TIM4 get 16 MHz. */
  RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2); /* RCC: bus prescalers */
  RCC->DCKCFGR &= ~RCC_DCKCFGR_TIMPRE;             /* RCC: timer clock */
  SystemCoreClock = HSI_VALUE;                    /* CMSIS variable: core clock */
}

static void SysTick_Init(void)
{
  SysTick->CTRL = 0U;                            /* SysTick: stop */
  SysTick->LOAD = (SystemCoreClock / SYSTICK_HZ) - 1U; /* SysTick: 1 ms period */
  SysTick->VAL = 0U;                             /* SysTick: current count */
  /* TICKINT = 0: no interrupt. */
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; /* SysTick: start, polled */
}

static void Delay_ms(uint32_t milliseconds)
{
  SysTick->VAL = 0U;                             /* SysTick: restart the count */
  while (milliseconds > 0U)
  {
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {} /* SysTick: wrap flag */
    --milliseconds;
  }
}

static void Servo_Init(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;             /* RCC: GPIOB clock */
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;              /* RCC: TIM4 clock */
  (void)RCC->AHB1ENR;                             /* RCC: read back the clock enable */
  (void)RCC->APB1ENR;                             /* RCC: read back the clock enable */
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM4RST;           /* RCC: reset TIM4 */
  RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM4RST;          /* RCC: release the reset */
  GPIOB->OTYPER &= ~GPIO_PIN_6;                   /* GPIOB: push-pull */
  GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDR_OSPEED6) | (GPIO_SPEED_FREQ_LOW << GPIO_OSPEEDR_OSPEED6_Pos); /* GPIOB: output speed */
  GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD6) | (GPIO_NOPULL << GPIO_PUPDR_PUPD6_Pos); /* GPIOB: no pull-up/down */
  GPIOB->AFR[0] = (GPIOB->AFR[0] & ~GPIO_AFRL_AFSEL6) | ((uint32_t)GPIO_AF2_TIM4 << GPIO_AFRL_AFSEL6_Pos); /* GPIOB: select AF2 */
  /* 16 MHz / 16 = 1 MHz: a 1 us tick; 20000 ticks = 50 Hz. */
  TIM4->CR1 = TIM_CR1_ARPE;      /* TIM4: buffered ARR */
  TIM4->PSC = (HSI_VALUE / SERVO_TIMER_HZ) - 1U;  /* TIM4: prescaler */
  TIM4->ARR = (SERVO_TIMER_HZ / SERVO_PWM_HZ) - 1U; /* TIM4: counter top */
  TIM4->CCR1 = SERVO_CENTER_US;                   /* TIM4: pulse width */
  TIM4->CCMR1 = (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1) | TIM_CCMR1_OC1PE; /* TIM4: PWM mode 1 (OC1M = 110), buffered CCR1 */
  TIM4->CCER = 0U;                               /* TIM4: output off */
  TIM4->EGR = TIM_EGR_UG;                         /* TIM4: force an update */
  TIM4->SR = 0U;                                 /* TIM4: clear the flags */
  TIM4->CNT = 0U;                                /* TIM4: current count */
  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6) | ((GPIO_MODE_AF_PP & GPIO_MODE) << GPIO_MODER_MODE6_Pos); /* GPIOB: alternate function mode */
  TIM4->CCER = TIM_CCER_CC1E; /* TIM4: CH1 output on, active high (CC1P = 0) */
  TIM4->CR1 |= TIM_CR1_CEN;                       /* TIM4: start the counter */
}

static void ADC1_Init(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;             /* RCC: GPIOA clock */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;              /* RCC: ADC1 clock */
  (void)RCC->AHB1ENR;                             /* RCC: read back the clock enable */
  (void)RCC->APB2ENR;                             /* RCC: read back the clock enable */
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0) | (GPIO_MODE_ANALOG << GPIO_MODER_MODE0_Pos); /* GPIOA: analog mode */
  GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | (GPIO_NOPULL << GPIO_PUPDR_PUPD0_Pos); /* GPIOA: no pull-up/down */
  RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;            /* RCC: reset the ADC */
  RCC->APB2RSTR &= ~RCC_APB2RSTR_ADCRST;           /* RCC: release the reset */
  /* PCLK2 /4 = 4 MHz; single conversion, right-aligned. */
  ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE) | ADC_CCR_ADCPRE_0; /* ADC: prescaler /4 */
  ADC1->CR1 = 0U;                                /* ADC1: 12-bit resolution */
  ADC1->CR2 = 0U;                                /* ADC1: single conversion mode */
  ADC1->SMPR1 = 0U;                              /* ADC1: sampling time CH10-18 */
  ADC1->SMPR2 = ADC_SMPR2_SMP0;                   /* ADC1: 480-cycle sampling */
  ADC1->SQR1 = 0U;                               /* ADC1: sequence length 1 */
  ADC1->SQR2 = 0U;                               /* ADC1: sequence SQ7-12 */
  ADC1->SQR3 = 0U;                               /* ADC1: channel 0 = PA0 */
  ADC1->SR = 0U;                                 /* ADC1: clear the flags */
  ADC1->CR2 = ADC_CR2_ADON;                       /* ADC1: power on */
  Delay_ms(1U);                                 /* Let the ADC settle. */
}

static uint16_t ADC1_Read(void)
{
  ADC1->SR = 0U;                                 /* ADC1: clear the flags */
  ADC1->CR2 |= ADC_CR2_SWSTART;                   /* ADC1: start a conversion */
  while ((ADC1->SR & ADC_SR_EOC) == 0U) {}        /* ADC1: wait for the end of conversion */
  return (uint16_t)(ADC1->DR & ADC_MAX_VALUE);    /* ADC1: the result */
}
