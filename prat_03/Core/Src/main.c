/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define DT_D1 50                       // delay do LED D1
#define DT_D2 100                      // delay do LED D2
#define DT_D3 150                      // delay do LED D3
#define DT_D4 200                      // delay do LED D4
#define PER_PB7 2000                   // tempo do ciclo (ms) do led PB7
#define PER_PB8 2000                   // tempo do ciclo (ms) do led PB8
#define PWM_ARR 100                    // resolução (num steps) do PWM
#define MAX_quem_pisca 4               // num max LEDs piscando...
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
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
//volatile uint_32_t lb12;
//volatile uint_32_t lb13;
//volatile uint_32_t lb14;
//volatile uint_32_t lb15;
//volatile uint_32_t lb7;
//volatile uint_32_t lb8;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // como não garante entrar nas FSM na primeira vez, garante LEDs apagados
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_12,
			          GPIO_PIN_SET); // desliga os LEDs
  // para iniciar vars de controle para máquinas de estados dos leds
    static enum {INI_D1, LIG_D1, DSLG_D1} sttD1=INI_D1; // var estados de D1
    static enum {INI_D2, LIG_D2, DSLG_D2} sttD2=INI_D2; // var estados de D2
    static enum {INI_D3, LIG_D3, DSLG_D3} sttD3=INI_D3; // var estados de D3
    static enum {INI_D4, LIG_D4, DSLG_D4} sttD4=INI_D4; // var estados de D4
    static enum {INI_D7, INC_D7, DEC_D7}  sttD7=INI_D7; // var estados de D7
    static enum {INI_D8, INC_D8, DEC_D8}  sttD8=INI_D8; // var estados de D8
  // para controlar vars tempos de entrada na rotina ON/OFF de cada LED
    uint32_t tin_D1=0, tin_D2=0, tin_D3=0, tin_D4=0,
    		 tin_D7=0, tin_D8=0, quem_pisca=1;
    uint16_t dc_D7 = 0;                // ini dc_D7 (brilho LED D7)
    uint16_t dc_D8 = PWM_ARR;          // ini dc_D8 com maximo
    int dt_D7 = PER_PB7/(2*PWM_ARR);   // calc delta tempo p/ PB7
    int dt_D8 = PER_PB8/(2*PWM_ARR);   // calc delta tempo p/ PB8 = PB7
    // inicializar o PWM do timer 4 ch 2 zerado
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // inicializa PWM no T4C2
    TIM4->CCR2 = dc_D7;                // inicia comparador PWM c/ 0
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3); // inicializa PWM no T4C3
    TIM4->CCR3 = dc_D8;                // inicia comparador PWM c/ ARR-1

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */
	// executa sempre: teste não deixa a var quem_pisca ser maior que o máximo
			if(quem_pisca > MAX_quem_pisca) quem_pisca=1;

	// tarefa #1 : controlar o estado do LED D1
		if (quem_pisca==1)
		{
			switch (sttD1)
			{
			  case INI_D1:                 // vai iniciar a máquina de estado
				tin_D1 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD1 = LIG_D1;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl o LED
				break;
			  case LIG_D1:                 // estado para ligar o LED
				if((HAL_GetTick()-tin_D1)>DT_D1) // se HAL_GetTick()-tin_D1 > DT_D1
				{
				  tin_D1 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD1 = DSLG_D1;         // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // ligaLED
				}
				break;
			  case DSLG_D1:                // estado para desligar o LED
				if((HAL_GetTick()-tin_D1)>DT_D1) // se HAL_GetTick()-tin_D1 > DT_D1
				{
				  tin_D1 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD1 = LIG_D1;          // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl LED
				  quem_pisca ++;
				}
				break;
			};
		}

		// tarefa #2 : controlar o estado do LED D2
		if (quem_pisca==2)
		{
			switch (sttD2)
			{
			  case INI_D2:                 // vai iniciar a máquina de estado
				tin_D2 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD2 = LIG_D2;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl o LED
				break;
			  case LIG_D2:                 // estado para ligar o LED
				if((HAL_GetTick()-tin_D2)>DT_D2) // se HAL_GetTick()-tin_D2 > DT_D2
				{
				  tin_D2 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD2 = DSLG_D2;         // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // ligaLED
				}
				break;
			  case DSLG_D2:                // estado para desligar o LED
				if((HAL_GetTick()-tin_D2)>DT_D2) // se HAL_GetTick()-tin_D2 > DT_D2
				{
				  tin_D2 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD2 = LIG_D2;          // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl LED
				  quem_pisca ++;
				}
				break;
			};
		}

	// tarefa #3 : controlar o estado do LED D3
		if (quem_pisca==3)
		{
			switch (sttD3)
			{
			  case INI_D3:                 // vai iniciar a máquina de estado
				tin_D3 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD3 = LIG_D3;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl o LED
				break;
			  case LIG_D3:                 // estado para ligar o LED
				if((HAL_GetTick()-tin_D3)>DT_D3) // se HAL_GetTick()-tin_D3 > DT_D3
				{
				  tin_D3 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD3 = DSLG_D3;         // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // ligaLED
				}
				break;
			  case DSLG_D3:                // estado para desligar o LED
				if((HAL_GetTick()-tin_D3)>DT_D3) // se HAL_GetTick()-tin_D3 > DT_D3
				{
				  tin_D3 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD3 = LIG_D3;          // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl LED
				  quem_pisca ++;
				}
				break;
			};
		}

	// tarefa #4 : controlar o estado do LED D4
		if (quem_pisca==4)
		{
			switch (sttD4)
			{
			  case INI_D4:                 // vai iniciar a máquina de estado
				tin_D4 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD4 = LIG_D4;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl o LED
				break;
			  case LIG_D4:                 // estado para ligar o LED
				if((HAL_GetTick()-tin_D4)>DT_D4) // se HAL_GetTick()-tin_D4 > DT_D4
				{
				  tin_D4 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD4 = DSLG_D4;         // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // ligaLED
				}
				break;
			  case DSLG_D4:                // estado para desligar o LED
				if((HAL_GetTick()-tin_D4)>DT_D4) // se HAL_GetTick()-tin_D4 > DT_D4
				{
				  tin_D4 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				  sttD4 = LIG_D4;          // muda o prox estado da máquina
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl LED
				  quem_pisca ++;
				}
				break;
			};
		}

	// tarefa #5 : controlar brilho (duty cycle = dc_D7) do LED PB7
			switch (sttD7)
			{
			  case INI_D7:                 // vai iniciar a máquina de estado
				tin_D7 = HAL_GetTick();    // tempo em que iniciou a tarefa
				sttD7 = INC_D7;            // prox estado da máquina
		        TIM4->CCR2 = 0;            // inicia val dc_D7 = 0
				break;
			  case INC_D7:                 // estado para incrementar PWM
				if((HAL_GetTick()-tin_D7)>dt_D7)  // se o milis-tin_D7 > dt_PD7
				{
				  tin_D7 = HAL_GetTick();  // tempo p/ prox mudança dc_D7
				  ++ dc_D7;                // incrementa dc_D7
				  if (dc_D7>=PWM_ARR) sttD7=DEC_D7;  // muda maq est p/ DEC_D7
		          TIM4->CCR2 = dc_D7;      // set comparador com valor dc_D7
				}
				break;
			  case DEC_D7:                 // estado para decrementar o PWM
				if((HAL_GetTick()-tin_D7)>dt_D7)  // se o milis-tin_D7 > dt_PD7
				{
				  tin_D7 = HAL_GetTick();  // tempo p/ prox mudança dc_D7
				  -- dc_D7;                // decrementa dc_D7
				  if (dc_D7 <= 0) sttD7=INC_D7;  // muda maq est p/ INC_D7
		          TIM4->CCR2 = dc_D7;      // set comparador com valor dc_D7
				}
				break;
			};
	// tarefa #6 : controle do duty-cycle do pino PB8
			switch (sttD8)
			{
			  case INI_D8:                 // vai iniciar a máquina de estado
				tin_D8 = HAL_GetTick();    // tempo em que iniciou a tarefa
				sttD8 = INC_D8;            // prox estado da máquina
		        TIM4->CCR3 = 0;            // inicia val dc_D8 = 0
				break;
			  case INC_D8:                 // estado para incrementar PWM
				if((HAL_GetTick()-tin_D8)>dt_D8)  // se o milis-tin_D8 > dt_PD8
				{
				  tin_D8 = HAL_GetTick();  // tempo p/ prox mudança dc_D8
				  ++ dc_D8;                // incrementa dc_D8
				  if (dc_D8>=PWM_ARR) sttD8=DEC_D8;  // muda maq est p/ DEC_D8
		          TIM4->CCR3 = dc_D8;      // set comparador com valor dc_D8
				}
				break;
			  case DEC_D8:                 // estado para decrementar o PWM
				if((HAL_GetTick()-tin_D8)>dt_D8)  // se o milis-tin_D8 > dt_PD8
				{
				  tin_D8 = HAL_GetTick();  // tempo p/ prox mudança dc_D8
				  -- dc_D8;                // decrementa dc_D8
				  if (dc_D8 <= 0) sttD8=INC_D8;  // muda maq est p/ INC_D8
		          TIM4->CCR3 = dc_D8;      // set comparador com valor dc_D8
				}
				break;
			};
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 359;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 99;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
