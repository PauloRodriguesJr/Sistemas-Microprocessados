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
#include "stm32f10x.h"
#include "stm32f1xx_it.h"      // prots serv interrupt (ISR, em stm32f1xx_it.c)
#include "prat_05_funcoes.h"   // header do arqv das funcoes do programa

#define FREQ_TICK 1000         // frequencia do tick = 1000 Hz (1ms)
#define DELAY_VARRE  7         // inc varredura a cada 7 ms (~142 Hz)
#define DIGITO_APAGADO 0x10    // kte valor p/ apagar um dígito no display
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_NVIC_Init(void);
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
  // funções mudadas para o arquivo "funcoes.c" para melhor organizar o projeto
  setup_RCC();                         // ini: habilitar CLKs dos periféricos
  setup_GPIOs();                       // setup GPIOs interface LEDs
  setup_INT_externa();                 // setup Interrupcao externa
  setup_ADC_conv();                    // setup do conversor ADC
  setup_systick(FREQ_TICK);            // set timers p/ 1 ms  (1000 Hz)
  reset_pin_GPIOs();                   // garante pinos GPIOs inicializados
  reset_modo_oper();                   // zera var modo_oper

  // vars e flags de controle do programa no superloop...
  int
  milADC = 0,                          // ini decimo de seg
  cenADC = 0,                          // ini unidade de seg
  decADC = 0,                          // ini dezena de seg
  uniADC = 0;                          // ini unidade de minuto

  int16_t
  val7seg = 0x00FF,                    // inicia 7-seg com 0xF (tudo apagado)
  serial_data = 0x01FF,                // dado a serializar (dig | val7seg)
  val_adc = 0;                         // valor lido no ADC

  uint32_t miliVolt = 0x0,             // val adc convertido p/ miliVolts
  tIN_varre = 0;                       // registra tempo última varredura

  // var de estado que controla a varredura (qual display é mostrado)
  static enum {DIG_UNI, DIG_DEC, DIG_CENS, DIG_MILS} sttVARRE=DIG_MILS;

  // entra no loop infinito
  while (1)
  {
  // tarefa #1: se (modo_oper=1) faz uma conversão ADC
	if (get_modo_oper()==1)
    {
	  GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);  // apaga o LED PC14
      GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);  // liga o LED PC13
      // dispara por software uma conversão ADC
	  ADC_SoftwareStartConvCmd(ADC1, ENABLE);    // habilita conversão ADC1
	  // espera que a conversão termine (polling)
	  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET ) {}
      //if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
	  //{
        val_adc = ADC_GetConversionValue(ADC1);  // lê último valor convertido
	    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);       // limpa flag no fim conversão
	    // ADC1->SR = ~(uint32_t)ADC_FLAG_EOC;   // limpa flag diretamente
	 // }
      miliVolt = val_adc*3300/4095;              // converte p/ mili Volts
      uniADC = miliVolt/1000;                    // define valor unidade V
      decADC = (miliVolt-uniADC*1000)/100;       // décimo de V
      cenADC = (miliVolt-uniADC*1000-decADC*100)/10;      // centésimo V
      milADC = miliVolt-uniADC*1000-decADC*100-cenADC*10; // milésimo V
      reset_modo_oper();                         // zera var modo_oper
      GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);// apaga o LED PC13
  	  GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);// liga o LED PC14
    }  // fim da tarefa #1 (conversão ADC se modo=1)

  // tarefa #2 FAZ SEMPRE: qdo milis() > DELAY_VARRE ms, desde a última mudança
    if (milis()-tIN_varre > DELAY_VARRE )        // se ++0,1s atualiza o display
    {
      switch(sttVARRE)                 // teste e escolha de qual DIG vai varrer
      {
      case DIG_MILS:
  	    {
		    sttVARRE = DIG_CENS;       // ajusta p/ prox digito
			serial_data = 0x0001;      // display #1
			val7seg = conv_7_seg(milADC);
			break;
		}
      case DIG_CENS:
        {
          sttVARRE = DIG_DEC;          // ajusta p/ prox digito
		  serial_data = 0x00002;       // display #2
		  if(cenADC>0 || decADC>0 || uniADC>0)
		  {
		    val7seg = conv_7_seg(cenADC);
		  } else {
			val7seg = conv_7_seg(DIGITO_APAGADO);
		  }
		  break;
		}
      case DIG_DEC:
    	{
		  sttVARRE = DIG_UNI;        // ajusta p/ prox digito
          serial_data = 0x0004;      // display #3
          if(decADC>0 || uniADC>0)
          {
            val7seg = conv_7_seg(decADC);
          } else {
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
          break;
		}
      case DIG_UNI:
    	{
          sttVARRE = DIG_MILS;       // ajusta p/ prox digito
          serial_data = 0x0008;      // display #3
          if(uniADC>0)
          {
            val7seg = conv_7_seg(uniADC);
            val7seg &=0x7FFF;        // liga o ponto decimal
          } else {
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
          break;
		}
      }
      tIN_varre = milis();             // tmp atual em que fez essa varredura
      serial_data |= val7seg;          // OR com val7seg = dado a serializar
      serializar(serial_data);         // serializa dado p/74HC595 (shift reg)
    }  // -- fim da tarefa #2, rotina que faz de varredura do display
  }    // -- fim do loop infinito
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* ADC1_2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
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
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_6|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 PB13 PB14
                           PB15 PB6 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_6|GPIO_PIN_9;
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
