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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CMP_MODO 0        // =1 inc modo_oper direto / =0 pede IRQ

#define PS 720               // delta t (ms) para piscar SUPER lento
#define PL 480               // delta t (ms) para piscar o led LENTO
#define PM 240               // delta t (ms) para piscar o led MÉDIO
#define PR 80                // delta t (ms) para piscar o led R�?PIDO

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t lb12;
volatile uint32_t lb13;
volatile uint32_t lb14;
volatile uint32_t lb15;
volatile uint32_t lpa1 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
int get_modo_oper(void);
void set_modo_oper(void);
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

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  // garantir que PC13 começa desligado
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  // para iniciar vars de controle para máquinas de estados dos leds
    static enum {INI_D1, LIG_D1, DSLG_D1} sttD1=INI_D1; // var estados de D1
    static enum {INI_D2, LIG_D2, DSLG_D2} sttD2=INI_D2; // var estados de D2
    static enum {INI_D3, LIG_D3, DSLG_D3} sttD3=INI_D3; // var estados de D3
    static enum {INI_D4, LIG_D4, DSLG_D4} sttD4=INI_D4; // var estados de D4
    static enum {INI_PA1, LIG_PA1, DSLG_PA1} sttPA1=INI_PA1; // var estados de PA1

  // para controlar vars tempos de entrada na rotina ON/OFF de cada LED
    uint32_t tin_D1=0, tin_D2=0, tin_D3=0, tin_D4=0, tin_PA1=0;
    uint32_t dt_D1=0, dt_D2=0, dt_D3=0, dt_D4=0, dt_PA1=PR; // proximos dt p/ cada LED
    int modo = get_modo_oper();          // inicia modo como 0

	// estrutura de dados para programar pedido de interrupção por software
  EXTI_HandleTypeDef hexti_1 = {};
  hexti_1.Line = EXTI_LINE_1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

// tarefa #0 (executa sempre): ajustar tempos de piscar dos LEDs
	modo = get_modo_oper();
	switch(modo)
	{
	  case 0:   // mo modo '0' ajusta dt_xx dos LEDs
		dt_D1 = PS;                   // modo 0 - dt_D1
		dt_D2 = PL;                   // modo 0 - dt_D2
		dt_D3 = PM;                   // modo 0 - dt_D3
		dt_D4 = PR;                   // modo 0 - dt_D4
		break;
	  case 1:   // mo modo '1' reajusta dt_xx dos LEDs
		dt_D1 = PL;                   // modo 1 - dt_D1
		dt_D2 = PM;                   // modo 1 - dt_D2
		dt_D3 = PR;                   // modo 1 - dt_D3
		dt_D4 = PS;                   // modo 1 - dt_D4
		break;
	  case 2:   // mo modo '2' reajusta dt_xx dos LEDs
		dt_D1 = PM;                   // modo 2 - dt_D1
		dt_D2 = PR;                   // modo 2 - dt_D2
		dt_D3 = PS;                   // modo 2 - dt_D3
		dt_D4 = PL;                   // modo 2 - dt_D4
		break;
	  case 3:   // mo modo '3' reajusta dt_xx dos LEDs
		dt_D1 = PR;                   // modo 3 - dt_D1
		dt_D2 = PS;                   // modo 3 - dt_D2
		dt_D3 = PL;                   // modo 3 - dt_D3
		dt_D4 = PM;                   // modo 3 - dt_D4
		break;
	}

// tarefa #1 : controlar o estado do LED D1
	switch (sttD1)
	{
	  case INI_D1:                     // vai iniciar a máquina de estado
		tin_D1 = HAL_GetTick();        // tempo inicial que iniciou a tarefa
		sttD1 = LIG_D1;                // prox estado da máquina
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desliga o LED
		break;
	  case LIG_D1:          // estado para ligar o LED
		if((HAL_GetTick()-tin_D1)>dt_D1) // se HAL_GetTick()-tin_D1 > dt_D1
		{
		  tin_D1 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD1 = DSLG_D1;             // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // liga o LED
		  lb15 = 0;
		}
		break;
	  case DSLG_D1:                    // estado para desligar o LED
		if((HAL_GetTick()-tin_D1)>dt_D1) // se HAL_GetTick()-tin_D1 > dt_D1
		{
		  tin_D1 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD1 = LIG_D1;              // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desliga o LED
		  lb15 = 1;
		}
		break;
	};

// tarefa #2 : controlar o estado do LED D2
	switch (sttD2)
	{
	  case INI_D2:                     // vai iniciar a máquina de estado
		tin_D2 = HAL_GetTick();        // tempo inicial que iniciou a tarefa
		sttD2 = LIG_D2;                // prox estado da máquina
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desliga o LED
		break;
	  case LIG_D2:          // estado para ligar o LED
		if((HAL_GetTick()-tin_D2)>dt_D2) // se HAL_GetTick()-tin_D2 > dt_D2
		{
		  tin_D2 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD2 = DSLG_D2;             // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // liga o LED
		  lb14 = 0;
		}
		break;
	  case DSLG_D2:                    // estado para desligar o LED
		if((HAL_GetTick()-tin_D2)>dt_D2) // se HAL_GetTick()-tin_D2 > dt_D2
		{
		  tin_D2 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD2 = LIG_D2;              // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desliga o LED
		  lb14 = 1;
		}
		break;
	};

// tarefa #3 : controlar o estado do LED D3
	switch (sttD3)
	{
	  case INI_D3:                     // vai iniciar a máquina de estado
		tin_D3 = HAL_GetTick();        // tempo inicial que iniciou a tarefa
		sttD3 = LIG_D3;                // prox estado da máquina
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desliga o LED
		break;
	  case LIG_D3:          // estado para ligar o LED
		if((HAL_GetTick()-tin_D3)>dt_D3) // se HAL_GetTick()-tin_D3 > dt_D3
		{
		  tin_D3 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD3 = DSLG_D3;             // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // liga o LED
		  lb13 = 0;
		}
		break;
	  case DSLG_D3:                    // estado para desligar o LED
		if((HAL_GetTick()-tin_D3)>dt_D3) // se HAL_GetTick()-tin_D3 > dt_D3
		{
		  tin_D3 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD3 = LIG_D3;              // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desliga o LED
		  lb13 = 1;
		}
		break;
	};

// tarefa #4 : controlar o estado do LED D4
	switch (sttD4)
	{
	  case INI_D4:                     // vai iniciar a máquina de estado
		tin_D4 = HAL_GetTick();        // tempo inicial que iniciou a tarefa
		sttD4 = LIG_D4;                // prox estado da máquina
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desliga o LED
		break;
	  case LIG_D4:          // estado para ligar o LED
		if((HAL_GetTick()-tin_D4)>dt_D4) // se HAL_GetTick()-tin_D4 > dt_D4
		{
		  tin_D4 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD4 = DSLG_D4;             // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // liga o LED
		  lb12 = 0;
		}
		break;
	  case DSLG_D4:                    // estado para desligar o LED
		if((HAL_GetTick()-tin_D4)>dt_D4) // se HAL_GetTick()-tin_D4 > dt_D4
		{
		  tin_D4 = HAL_GetTick();	   // guarda tempo p/ prox mudança estado
		  sttD4 = LIG_D4;              // muda o prox estado da máquina
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desliga o LED
		  lb12 = 1;
		}
		break;
	};

// tarefa #5 - com interrupção gerada por software
#if CMP_MODO==0                        // DIRETIVA PARA O COMPILADOR !!!
		 // --- tarefa #5A - XMODO = 0   (com interrupção gerada por software)
		  switch (sttPA1)
		  {
		  case INI_PA1:                    // vai iniciar a máquina de estado
		    sttPA1 = LIG_PA1;              // prox estado da máquina
		    break;
		  case LIG_PA1:                    // estado p/ gerar INT_EXT_1
		    if((HAL_GetTick()-tin_PA1)>dt_PA1) // se HAL_GetTick()-tin_PA1 > dt_PA1
		    {
		      tin_PA1 = HAL_GetTick();     // guarda tempo p/ prox mudança estado
		      if (lpa1 == 1) {
		        sttPA1 = DSLG_PA1;             // muda o prox estado da máquina
		        //HAL_EXTI_ClearPending(&hexti_1, EXTI_TRIGGER_FALLING);
		        HAL_EXTI_GenerateSWI(&hexti_1);  // pedido de int por software
		        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // ligar led
				lpa1=0;                    // volta lpa1 para desativada
		      }
		    }
		    break;
		  case DSLG_PA1:                   // estado para desligar o LED
		    if((HAL_GetTick()-tin_PA1)>dt_PA1) // se HAL_GetTick()-tin_PA1 > dt_PA1
		    {
			  tin_PA1 = HAL_GetTick();     // guarda tempo p/ prox mudança estado
		      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // ligar led
		      sttPA1 = LIG_PA1;            // muda o prox estado da máquina
		    }
		    break;
		  };
      // -- final da tarefa 5A ...
#else                                  // DIRETIVA PARA O COMPILADOR !!!
		  // --- tarefa #5B - MODO = 1   (com interrupção gerada pelo modo)
		  switch (sttPA1)
		  {
		  case INI_PA1:                    // vai iniciar a máquina de estado
		    sttPA1 = LIG_PA1;              // prox estado da máquina
		    break;
		  case LIG_PA1:                    // estado para inc modo_oper
		    if((HAL_GetTick()-tin_PA1)>dt_PA1) // se HAL_GetTick()-tin_D4 > dt_D4
		    {
		      tin_PA1 = HAL_GetTick();     // guarda tempo p/ prox mudança estado
		      if (lpa1==1)
		      {
		        sttPA1 = DSLG_PA1;         // muda o prox estado da máquina
			    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // ligar led
		        set_modo_oper();          // set do modo atual
		      }
		    }
		    break;
		  case DSLG_PA1:                   // estado para desligar o LED
		    if((HAL_GetTick()-tin_PA1)>dt_PA1) // se HAL_GetTick()-tin_D4 > dt_D4
		    {
		      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // desligar led
		      tin_PA1 = HAL_GetTick();     // guarda tempo p/ prox mudança estado
		      sttPA1 = LIG_PA1;            // muda o prox estado da máquina
		      lpa1=0;                      // volta lpa1=0 para desativar
		    }
		    break;
		  };
    // -- final da tarefa 5
#endif                                 // FIM DA DIRETIVA PARA O COMPILADOR !!!
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
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
