/**
  ******************************************************************************
  * @file    stm32f1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-February-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                          */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

// define a variavel "ticks" (conta a quantidade de INT temporal do sistema)
static uint32_t volatile ticks;        // var que conta num ticks do sistema
// rotina de tratamento da interrupcao (ISR) to timer SysTick
void SysTick_Handler(void)
{
  ++ ticks;
}

// funcao que retorna o valor da variavel "ticks"
uint32_t milis(void)
{
  static uint32_t numTicks;            // var local recebe num ticks
// OBS: seção crítica, desabilitamos todas as IRQs p/ atualizar var
  __disable_irq();                     // desabilita IRQs
  numTicks = ticks;                    // faz numTicks = ticks
  __enable_irq();                      // volta habilitar IRQs

  return numTicks;                     // retorna numTicks até agora
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

// ISR rotina de servico de IRQ linha 1  (Joao Ranhel - 09/2017)
static uint32_t volatile modo_oper=0;  // define var modo_oper
static uint32_t tin_IRQ6 = 0;          // define var tempo p/ prox
void EXTI9_5_IRQHandler(void)
{
  if ((milis()-tin_IRQ6)>DT_DEBOUNCING)// se já deu tempo
  {
    if (EXTI_GetITStatus(EXTI_Line6)!=RESET)// se foi IRQ6
    {
      modo_oper ++;                    // inc modo operacao
      if (modo_oper>MAX_MOD_OPER) modo_oper=0;// se >MAX modo_oper=0
      tin_IRQ6 = milis();              // ultimo tempo que atendeu IRQ6
    }
  }
  EXTI_ClearITPendingBit(EXTI_Line6);
}

// fn que qpenas retorna o valor da var modo_oper
int get_modo_oper(void){
  static int x;                        // var local recebe modo_oper
  // OBS: seção crítica, desabilitamos todas as IRQs p/ atualizar var
  __disable_irq();                     // desabilita IRQs
  x = modo_oper;                       // faz x = modo_oper
  __enable_irq();                      // volta habilitar IRQs
  return x;                            // retorna x (=modo_oper)
}

//fn que reseta a variável modo_oper (volta a var para zero)
void reset_modo_oper(void){
  // OBS: seção crítica, desabilitamos todas as IRQs p/ atualizar var
  __disable_irq();                     // desabilita IRQs
  modo_oper = 0;                       // faz modo_oper=0
  __enable_irq();                      // volta habilitar IRQs
}
