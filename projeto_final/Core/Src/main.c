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

#define PR 80
#define DT_D1 200                      // delay do LED D1
#define DT_D2 300                      // delay do LED D2
#define DT_D3 400                      // delay do LED D3
#define DT_D4 500                      // delay do LED D4
#define DT_ALL 500                      // delay do LED D4
#define PER_PWM 3000                   // tempo do ciclo (ms) do led PB7
#define PWM_ARR 200                    // resolução (num steps) do PWM
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
volatile uint32_t mn_led1 = 0;
volatile uint32_t mn_led2 = 0;
volatile uint32_t mn_led3 = 0;
volatile uint32_t mn_led4 = 0;
volatile uint32_t mn_dt_PWM;
volatile uint32_t mn_tin_CRON = 0;
volatile uint32_t lpa1 = 0;
volatile uint32_t lpa2 = 0;
volatile uint32_t lpa3 = 0; // variável utilizada para simular efeito de borda causado por botão do Monitor
//volatile uint32_t clear_signal = 0; // variável utilizada para simular efeito de borda causado por botão do Monitor

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
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
int main(void) {
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

	/* Initialize interrupts */
	MX_NVIC_Init();
	/* USER CODE BEGIN 2 */

	// garantir que PC13 começa desligado
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	// para iniciar vars de controle para máquinas de estados dos leds
	static enum {
		INI_D1, LIG_D1, DSLG_D1} sttD1 = INI_D1; // var estados de D1
	static enum {
		INI_D2, LIG_D2, DSLG_D2} sttD2 = INI_D2; // var estados de D2
	static enum {
		INI_D3, LIG_D3, DSLG_D3} sttD3 = INI_D3; // var estados de D3
	static enum {
		INI_D4, LIG_D4, DSLG_D4} sttD4 = INI_D4; // var estados de D4
	static enum {
		INI_ALL, LIG_ALL, DSLG_ALL} sttALL = INI_ALL; // var estados dos 4 LEDs piscando juntos
	// maquina de estados das interrupções geradas por software
	static enum {
		INI_PA1, LIG_PA1, DSLG_PA1} sttPA1 = INI_PA1; // var estados de PA1
	static enum {
		INI_PA2, LIG_PA2, DSLG_PA2} sttPA2 = INI_PA2; // var estados de PA2
    static enum {
    	INI_PA3, LIG_PA3, DSLG_PA3} sttPA3=INI_PA3;   // var estados de PA3

	static enum {
		INI_PWM, INC_PWM, DEC_PWM} sttGAUGE = INI_PWM; // var de estados do gauge PWM
	static enum {
		INI_CRON, INC_CRON, PAUSE_CRON} sttCRON = INI_CRON; // var estados do cronometro

	// para controlar vars tempos de entrada na rotina ON/OFF de cada LED
	uint32_t tin_D1 = 0, tin_D2 = 0, tin_D3 = 0, tin_D4 = 0, tin_PWM = 0;
	uint32_t tin_CRON = 0;
	uint32_t dt_PA1 = PR, dt_PA2 = PR,  dt_PA3 = PR; // proximos dt p/ cada PA

	uint32_t tin_PA1 = 0, tin_PA2 = 0, tin_PA3 = 0; // dts das interrupções por software
	mn_tin_CRON = 0;

	int dt_PWM = PER_PWM / (2 * PWM_ARR);   // calc delta tempo p/ PB7

	int modo = get_modo_oper();          // inicia modo como 0
	int start_status = get_cron_start_status();
	int clear_signal = get_cron_clear_status();
	uint16_t dc_PWM = 0;

	// inicializar o PWM do timer 4 ch 2 zerado
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // inicializa PWM no T4C2
	TIM4->CCR2 = dc_PWM;                // inicia comparador PWM c/ 0

	// estrutura de dados para programar pedido de interrupção por software
	EXTI_HandleTypeDef hexti_1 = { };
	hexti_1.Line = EXTI_LINE_1;
	EXTI_HandleTypeDef hexti_2 = { };
	hexti_2.Line = EXTI_LINE_2;
    EXTI_HandleTypeDef hexti_3 = {};
    hexti_3.Line = EXTI_LINE_3;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		modo = get_modo_oper();

		if (modo == 0) {
			// Caso pisca independentes:
			// Cada LED piscando com um periodo especifico DT_Di, i={1,2,3,4}

			// controle do estado do LED D1
			switch (sttD1) {
			case INI_D1:                 // vai iniciar a máquina de estado
				tin_D1 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD1 = LIG_D1;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl o LED
				mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
				break;
			case LIG_D1:                 // estado para ligar o LED
				if ((HAL_GetTick() - tin_D1) > DT_D1) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D1 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD1 = DSLG_D1;         // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // ligaLED
					mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
				}
				break;
			case DSLG_D1:                // estado para desligar o LED
				if ((HAL_GetTick() - tin_D1) > DT_D1) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D1 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD1 = LIG_D1;          // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl LED			}
					mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
				}
				break;
			};

			// controle do estado do LED D2
			switch (sttD2) {
			case INI_D2:                 // vai iniciar a máquina de estado
				tin_D2 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD2 = LIG_D2;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl o LED
				mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
				break;
			case LIG_D2:                 // estado para ligar o LED
				if ((HAL_GetTick() - tin_D2) > DT_D2) // se HAL_GetTick()-tin_D2 > DT_D2
				{
					tin_D2 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD2 = DSLG_D2;         // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // ligaLED
					mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
				}
				break;
			case DSLG_D2:                // estado para desligar o LED
				if ((HAL_GetTick() - tin_D2) > DT_D2) // se HAL_GetTick()-tin_D2 > DT_D2
				{
					tin_D2 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD2 = LIG_D2;          // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl LED
					mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
				}
				break;
			};

			// controle do estado do LED D3
			switch (sttD3) {
			case INI_D3:                 // vai iniciar a máquina de estado
				tin_D3 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD3 = LIG_D3;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl o LED
				mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
				break;
			case LIG_D3:                 // estado para ligar o LED
				if ((HAL_GetTick() - tin_D3) > DT_D3) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D3 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD3 = DSLG_D3;         // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // ligaLED
					mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
				}
				break;
			case DSLG_D3:                // estado para desligar o LED
				if ((HAL_GetTick() - tin_D3) > DT_D3) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D3 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD3 = LIG_D3;          // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl LED
					mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
				}
				break;
			};

			// controle do estado do LED D4
			switch (sttD4) {
			case INI_D4:                 // vai iniciar a máquina de estado
				tin_D4 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttD4 = LIG_D4;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl o LED
				mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
				break;
			case LIG_D4:                 // estado para ligar o LED
				if ((HAL_GetTick() - tin_D4) > DT_D4) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D4 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD4 = DSLG_D4;         // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // ligaLED
					mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
				}
				break;
			case DSLG_D4:                // estado para desligar o LED
				if ((HAL_GetTick() - tin_D4) > DT_D4) // se HAL_GetTick()-tin_D1 > DT_D1
				{
					tin_D4 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttD4 = LIG_D4;          // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl LED
					mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
				}
				break;
			};

		} else {
			// Caso pisca todos juntos:
			// Todos os LEDs piscando em sincronia com período = DT_ALL
			// OBS: Usando o tin_D4 por conveniência apenas, mas poderia ser tin_ALL
			switch (sttALL) {
			case INI_ALL:                 // vai iniciar a máquina de estado
				tin_D4 = HAL_GetTick();    // tempo inicial que iniciou a tarefa
				sttALL = LIG_ALL;            // prox estado da máquina
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl o LED1
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl o LED2
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl o LED3
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl o LED4

				mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED1
				mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED2
				mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13); //  le estado LED3
				mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12); //  le estado LED4
				break;
			case LIG_ALL:                 // estado para ligar os LEDs
				if ((HAL_GetTick() - tin_D4) > DT_ALL) // se HAL_GetTick()-tin_D4 > DT_ALL
				{
					tin_D4 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttALL = DSLG_ALL;         // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // lig o LED1
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // lig o LED2
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // lig o LED3
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // lig o LED4

					mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED1
					mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED2
					mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13); //  le estado LED3
					mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12); //  le estado LED4
				}
				break;
			case DSLG_ALL:                // estado para desligar os LEDs
				if ((HAL_GetTick() - tin_D4) > DT_ALL) // se HAL_GetTick()-tin_D4 > DT_ALL
				{
					tin_D4 = HAL_GetTick(); // guarda tempo p/ prox mudança estado
					sttALL = LIG_ALL;          // muda o prox estado da máquina
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desl o LED1
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desl o LED2
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desl o LED3
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desl o LED4

					mn_led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED1
					mn_led2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //  le estado LED2
					mn_led3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13); //  le estado LED3
					mn_led4 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12); //  le estado LED4
				}
				break;
			};

		}

// 		Interrupção gerada por software: lpa1

		switch (sttPA1) {
		case INI_PA1:                    // vai iniciar a máquina de estado
			sttPA1 = LIG_PA1;              // prox estado da máquina
			break;
		case LIG_PA1:                    // estado p/ gerar INT_EXT_1
			if ((HAL_GetTick() - tin_PA1) > dt_PA1) // se HAL_GetTick()-tin_PA1 > dt_PA1
			{
				tin_PA1 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				if (lpa1 == 1) {
					sttPA1 = DSLG_PA1;          // muda o prox estado da máquina
					HAL_EXTI_ClearPending(&hexti_1, EXTI_TRIGGER_FALLING);
					HAL_EXTI_GenerateSWI(&hexti_1); // pedido de int por software
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET); // ligar led
					lpa1 = 0;                    // volta lpa1 para desativada
				}
			}
			break;
		case DSLG_PA1:                   // estado para desligar o LED
			if ((HAL_GetTick() - tin_PA1) > dt_PA1) // se HAL_GetTick()-tin_PA1 > dt_PA1
					{
				tin_PA1 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET); // ligar led
				sttPA1 = LIG_PA1;            // muda o prox estado da máquina
			}
			break;
		};

// 		Interrupçao gerada por software :lpa2
		switch (sttPA2) {
		case INI_PA2:                    // vai iniciar a máquina de estado
			sttPA2 = LIG_PA2;              // prox estado da máquina
			break;
		case LIG_PA2:                    // estado p/ gerar INT_EXT_1
			if ((HAL_GetTick() - tin_PA2) > dt_PA2) // atraso para resposta do sinal
					{
				tin_PA2 = HAL_GetTick(); // incrementa contador de atraso de resposta
				if (lpa2 == 1) {
					sttPA2 = DSLG_PA2;          // muda o prox estado da máquina
					HAL_EXTI_ClearPending(&hexti_2, EXTI_TRIGGER_FALLING);
					HAL_EXTI_GenerateSWI(&hexti_2); // pedido de int por software
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET); // ligar led
					lpa2 = 0;                    // volta lpa2 para desativada
				}
			}
			break;
		case DSLG_PA2:                   // estado para desligar o LED
			if ((HAL_GetTick() - tin_PA2) > dt_PA2) // se HAL_GetTick()-tin_PA1 > dt_PA1
			{
				tin_PA2 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET); // ligar led
				sttPA2 = LIG_PA2;            // muda o prox estado da máquina
			}
			break;
		};

		// 		Interrupçao gerada por software :lpa3
				switch (sttPA3) {
				case INI_PA3:                    // vai iniciar a máquina de estado
					sttPA3 = LIG_PA3;              // prox estado da máquina
					break;
				case LIG_PA3:                    // estado p/ gerar INT_EXT_3
					if ((HAL_GetTick() - tin_PA3) > dt_PA3) // atraso para resposta do sinal
							{
						tin_PA3 = HAL_GetTick(); // incrementa contador de atraso de resposta
						if (lpa3 == 1) {
							sttPA3 = DSLG_PA3;          // muda o prox estado da máquina
							HAL_EXTI_ClearPending(&hexti_3, EXTI_TRIGGER_FALLING);
							HAL_EXTI_GenerateSWI(&hexti_3); // pedido de int por software
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET); // ligar led
							lpa3 = 0;                    // volta lpa2 para desativada
						}
					}
					break;
				case DSLG_PA3:                   // estado para desligar o LED
					if ((HAL_GetTick() - tin_PA3) > dt_PA3) // se HAL_GetTick()-tin_PA1 > dt_PA3
					{
						tin_PA3 = HAL_GetTick();  // guarda tempo p/ prox mudança estado
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET); // ligar led
						sttPA3 = LIG_PA3;            // muda o prox estado da máquina
					}
					break;
				};

		// CONTROLE DO CRONOMETRO
		// Maquina de estados do cronometro -
		// Estado INI : Zera o cronometro e mantém o track do GetTick
		// Estado INC : Incrementa "diferenciais" de tempo no cronometro (em ms)
		// Estado PAUSE: Mantem o track do GetTick, mas para de incrementar
		switch (sttCRON) {
		case INI_CRON:					// estado que vai limpar o cronometro
			mn_tin_CRON = 0;
			tin_CRON = HAL_GetTick();    // tempo em que iniciou a tarefa
			break;
		case INC_CRON:            // estado para incrementar tempo do cronometro
			// Calcula o diferencial de tempo da ultima entrada e incrementa no cronometro
			mn_tin_CRON = mn_tin_CRON + (HAL_GetTick() - tin_CRON); // Vai somando infinitamente em (ms)
			tin_CRON = HAL_GetTick();    // tempo em que iniciou a tarefa
			break;
		case PAUSE_CRON:
			tin_CRON = HAL_GetTick();    // Manter track do HAL_GetTick
			break;
		};

		// Definição dos estados da Maq Cronometro
		clear_signal = get_cron_clear_status();
		start_status = get_cron_start_status();
		if (start_status == 0) {
			if (clear_signal == 1) {
				sttCRON = INI_CRON; // Volta para o INI_CRON, que limpa o contador do cronometro
//				clear_signal = 0;
				reset_cron_clear_status();
			} else {
				sttCRON = PAUSE_CRON;
			}
		} else {
			sttCRON = INC_CRON;
		}

		//
		// Máquina de Estados para Controle do Gauge de PWM
		switch (sttGAUGE) {
		case INI_PWM:                 // vai iniciar a máquina de estado
			tin_PWM = HAL_GetTick();    // tempo em que iniciou a tarefa
			sttGAUGE = INC_PWM;            // prox estado da máquina
			TIM4->CCR2 = 0;            // inicia val dc_PWM= 0
			mn_dt_PWM = dc_PWM;
			break;
		case INC_PWM:                 // estado para incrementar PWM
			if ((HAL_GetTick() - tin_PWM) > dt_PWM) // se o milis-tin_PWM > dt_PWM
			{
				tin_PWM = HAL_GetTick();  // tempo p/ prox mudança dc_PWM
				++dc_PWM;                // incrementa dc_PWM
				if (dc_PWM >= PWM_ARR)
					sttGAUGE = DEC_PWM;  // muda maq est p/ DEC_PWM
				TIM4->CCR2 = dc_PWM;      // set comparador com valor dc_PWM
				mn_dt_PWM = dc_PWM;
			}
			break;
		case DEC_PWM:                 // estado para decrementar o PWM
			if ((HAL_GetTick() - tin_PWM) > dt_PWM) // se o milis-tin_PWM > dt_PWM
			{
				tin_PWM = HAL_GetTick();  // tempo p/ prox mudança dc_PWM
				--dc_PWM;                // decrementa dc_PWM
				if (dc_PWM <= 0)
					sttGAUGE = INC_PWM;  // muda maq est p/ INC_PWM
				TIM4->CCR2 = dc_PWM;      // set comparador com valor dc_PWM
				mn_dt_PWM = dc_PWM;
			}
			break;
		};
	} // EndWhile
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief NVIC Configuration.
 * @retval None
 */
static void MX_NVIC_Init(void) {
	/* EXTI1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 0;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
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
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PA1 PA2 PA3 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB12 PB13 PB14 PB15 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
