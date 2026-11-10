/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button_fsm.h"
#include "load_driver.h"
#include "pot_sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
/* The four FSM buttons; the crash sensor is not here, it has its own interrupt */
enum { IDX_LOCK, IDX_LEFT, IDX_RIGHT, IDX_HAZARD, N_FSM_BUTTONS };
static button_fsm_t s_buttons[N_FSM_BUTTONS];

/* The car's state: plain flags are enough here */
static bool s_locked = false;
static bool s_valet_mode = false;
static bool s_turn_l_on = false;
static bool s_turn_r_on = false;
static bool s_hazard_on = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* The glue: the only place where the buttons meet the loads. One small handler per button;
   each ignores BTN_EVENT_NONE, because no branch matches it */
static void handle_lock_button(button_event_t ev)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		s_locked = !s_locked;
		load_relay_set(s_locked); /* the relay = the door lock */
		load_buzzer_chirp(s_locked ? 80u : 40u); /* a different beep for lock and unlock */
	} else if (ev == BTN_EVENT_LONG_PRESS) {
		s_valet_mode = !s_valet_mode;
		load_buzzer_chirp(300u);
	}
}

/* left = true for LEFT, false for RIGHT. One side at a time: switching one on switches the other off */
static void handle_turn_button(button_event_t ev, bool left)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		if (left) {
			s_turn_l_on = !s_turn_l_on;
			if (s_turn_l_on) {
				s_turn_r_on = false;
			}
		} else {
			s_turn_r_on = !s_turn_r_on;
			if (s_turn_r_on) {
				s_turn_l_on = false;
			}
		}
	}
	/* LONG = the lane change (3 blinks): challenge 1, nothing yet */
}

static void handle_hazard_button(button_event_t ev)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		s_hazard_on = !s_hazard_on;
	} else if (ev == BTN_EVENT_LONG_PRESS) {
		load_relay_pulse(200u); /* diagnostics: the relay and the buzzer together for 200 ms */
		load_buzzer_chirp(200u);
	}
}

/* The crash is a safety event: it wins over the user's buttons */
static void handle_crash(void)
{
	s_hazard_on = true;
	load_relay_pulse(150u);
	load_buzzer_chirp(150u);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  load_driver_init();
  button_fsm_init(&s_buttons[IDX_LOCK], BTN_LOCK_GPIO_Port, BTN_LOCK_Pin); /* &: pass the struct's address, init fills it */
  button_fsm_init(&s_buttons[IDX_LEFT], BTN_LEFT_GPIO_Port, BTN_LEFT_Pin);
  button_fsm_init(&s_buttons[IDX_RIGHT], BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin);
  button_fsm_init(&s_buttons[IDX_HAZARD], BTN_HAZARD_GPIO_Port, BTN_HAZARD_Pin);
  uint32_t last_fsm_ms = 0; /* HAL_GetTick() when each superloop slot ran last */
  uint32_t last_pot_ms = 0;
  uint32_t last_blink_ms = 0;
  bool blink_phase = false; /* the flasher's half period: the turn LEDs on or off */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint32_t now = HAL_GetTick(); /* ms since start; each slot below runs when its period has passed, nothing waits */

	  if (now - last_fsm_ms >= BTN_FSM_TICK_MS) { /* every 10 ms: the buttons; unsigned subtraction survives the wrap */
		  last_fsm_ms = now;
		  handle_lock_button(button_fsm_tick(&s_buttons[IDX_LOCK], now));
		  handle_turn_button(button_fsm_tick(&s_buttons[IDX_LEFT], now), true);
		  handle_turn_button(button_fsm_tick(&s_buttons[IDX_RIGHT], now), false);
		  handle_hazard_button(button_fsm_tick(&s_buttons[IDX_HAZARD], now));

		  /* The state on the LEDs instead of the PDF's status log: the turn LEDs blink in the flasher's phase */
		  load_led_left_set(blink_phase && (s_hazard_on || s_turn_l_on)); /* the hazard wins; the turn flag stays and comes back */
		  load_led_right_set(blink_phase && (s_hazard_on || s_turn_r_on));
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, s_valet_mode ? GPIO_PIN_RESET : GPIO_PIN_SET); /* valet; lit on LOW */
	  }

	  if (now - last_pot_ms >= POT_SAMPLE_MS) { /* every 50 ms: the potentiometer */
		  last_pot_ms = now;
		  pot_sensor_update();
	  }

	  /* The flasher: the pot sets the period, 200..800 ms (integer map, no float); 4095 * 600 fits in 32 bits */
	  uint32_t blink_period_ms = 200u + ((uint32_t)pot_sensor_get_raw() * 600u) / 4095u;
	  if (now - last_blink_ms >= blink_period_ms / 2u) { /* every half period the LEDs and the click change */
		  last_blink_ms = now;
		  bool blinking = s_hazard_on || s_turn_l_on || s_turn_r_on;
		  blink_phase = blinking ? !blink_phase : false;
		  load_buzzer_tick(blink_phase); /* tick on, tock off: each change clicks the passive buzzer */
	  }

	  if (crash_sensor_take()) { /* every pass: the interrupt already caught it, only react here */
		  handle_crash();
	  }

	  load_driver_update(); /* every pass: ends a beep or a relay pulse whose time is up */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1249;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RELAY_Pin|LED_LEFT_Pin|LED_RIGHT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RELAY_Pin LED_LEFT_Pin LED_RIGHT_Pin */
  GPIO_InitStruct.Pin = RELAY_Pin|LED_LEFT_Pin|LED_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_LOCK_Pin BTN_LEFT_Pin BTN_RIGHT_Pin BTN_HAZARD_Pin */
  GPIO_InitStruct.Pin = BTN_LOCK_Pin|BTN_LEFT_Pin|BTN_RIGHT_Pin|BTN_HAZARD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CRASH_Pin */
  GPIO_InitStruct.Pin = CRASH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(CRASH_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
