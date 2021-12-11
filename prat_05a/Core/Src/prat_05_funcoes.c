/* ----------------------------------------------------------------------------
 UFABC - Disciplina Sistemas Microprocessados - SuP - 2018.10
 Programa: pPrat_05 - arquivo de funções: prat_05_funcoes.c
 Autor:     Joao Ranhel
 Descricao: contém as funções para criar o programa CONVERSOR ADC prat_05
 Usa:  arquivo 'prat_05_funcoes.h" que contém os #defines e protótipos
       de todas as funções descritas aqui
/  --------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "prat_05_funcoes.h"

// -- (parte a) -- config de periféricos
// estas funções fazem o setup dos periféricos- estão prototipadas antes do main()
// configurar CLKs dos periféricos
void setup_RCC(void)
{
  // habilitar o clock no barramento das GPIOs B e C
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  // habilitar o clock do periferico de interrupcao
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  // habilitar o clock do conversor ADC1 no periférico APB2
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

// fn cuida do SETUP das GPIOs C e B
void setup_GPIOs(void)
{
  GPIO_InitTypeDef GPIO_str;   // estrutura dados config GPIO_C e GPIO_B
  // setup dos vals pars da estrutura de dados da GPIO_C e iniciar
  GPIO_str.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_str.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_str.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOC, &GPIO_str);

  // setup dos vals pars da estrutura de dados da GPIO_B e iniciar
  GPIO_str.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_1 | GPIO_Pin_0;
  GPIO_str.GPIO_Mode = GPIO_Mode_Out_PP;  // modo de OUT
  GPIO_str.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOB, &GPIO_str);

  // setup dos vals p/ estrutura de GPIO_B e iniciar pino 6 INPUT
  GPIO_str.GPIO_Pin = GPIO_Pin_6;
  GPIO_str.GPIO_Mode = GPIO_Mode_IN_FLOATING; // modo input pino B6
  GPIO_str.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_str);

  // config que GPIOB pino 6 sera' usado para gerar EXT INT
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
  EXTI_ClearITPendingBit(EXTI_Line6);
}

// -- esta secao e nova nesta pratica --
void setup_INT_externa(void)
{
  EXTI_InitTypeDef IntExt_str; // declara estrut IntExt_str
  NVIC_InitTypeDef NVIC_str;   // estrutura NVIC port B
  // configurar dados estrutura interrupcao
  IntExt_str.EXTI_Line = EXTI_Line6;             // qual linha pede interrupcao
  IntExt_str.EXTI_Mode = EXTI_Mode_Interrupt;    // modo interrupcao
  IntExt_str.EXTI_Trigger = EXTI_Trigger_Rising; // dispara no falling_edge
  IntExt_str.EXTI_LineCmd = ENABLE;              // habilita ext_int
  EXTI_Init(&IntExt_str);  // chama funcao que inicializa interrupcao

  // configurar o NVIC (estrutura e funcao no misc.h e misc.c)
  NVIC_str.NVIC_IRQChannel = EXTI9_5_IRQn;       // IRQ_ext linha EXTI9_5_IRQn
  NVIC_str.NVIC_IRQChannelPreemptionPriority = 1;// prioridade preempt
  NVIC_str.NVIC_IRQChannelSubPriority = 1;       // prioridade 1
  NVIC_str.NVIC_IRQChannelCmd = ENABLE;		     // habilitada
  NVIC_Init(&NVIC_str);    // chama fn que inicializa NVIC
}

// fn de setup dos conversores ADC
void setup_ADC_conv(void)
{
  ADC_InitTypeDef ADC_str;       // estrutura de dados ADC
  ADC_str.ADC_Mode = ADC_Mode_Independent;
  ADC_str.ADC_ScanConvMode = DISABLE;
  ADC_str.ADC_ContinuousConvMode = DISABLE;
  ADC_str.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_str.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_str.ADC_NbrOfChannel = 0x1;
  ADC_Init(ADC1, &ADC_str);   // fn inicial ADC1 com base na structure acima
  // fn que configura o canal 0, rank, num clocks para conversão
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
  // fn que habilita o ADC1 para funcionamento
  ADC_Cmd(ADC1, ENABLE);        // habilita ADC1
}

// fn que configura o SysTick c/ 1ms  (freq_systema/frequency
void setup_systick(uint16_t f_tick)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  (void) SysTick_Config(RCC_Clocks.HCLK_Frequency / f_tick);
}
// -- (parte a) -- fim do config de periféricos

// -- (parte b) -- demais funções do programa
/* ------ FUNCAO que converte um inteiro hexadecimal em 7 segmentos --------
argmento:  NumHex(valor hexa) + kte TIPO_DISPLAY="0" anodo comum
Author: Joao Ranhel
Ordem dos bits no registrador de deslocamento:
dp g f e d c b a 0 0 0 0 0 0 0 0       (fara' um OR no retorno)
OBS: esta rotina nao liga  o DP...
 ----------------------------------------------------------------------------*/
int16_t conv_7_seg(int NumHex)
{
  int16_t sseg = 0xFF00;
  switch(NumHex)                     // valores default p/ ANODO comum
  {
    case 0: {sseg = 0xC000; break;}  // retorna val p/ 0
    case 1: {sseg = 0xF900; break;}  // retorna val p/ 1
    case 2: {sseg = 0xA400; break;}  // retorna val p/ 2
    case 3: {sseg = 0xB000; break;}  // retorna val p/ 3
    case 4: {sseg = 0x9900; break;}  // retorna val p/ 4
    case 5: {sseg = 0x9200; break;}  // retorna val p/ 5
    case 6: {sseg = 0x8200; break;}  // retorna val p/ 6
    case 7: {sseg = 0xF800; break;}  // retorna val p/ 7
    case 8: {sseg = 0x8000; break;}  // retorna val p/ 8
    case 9: {sseg = 0x9000; break;}  // retorna val p/ 9
    case 10: {sseg = 0x8800; break;} // retorna val p/ A
    case 11: {sseg = 0x8300; break;} // retorna val p/ B
    case 12: {sseg = 0xC600; break;} // retorna val p/ C
    case 13: {sseg = 0xA100; break;} // retorna val p/ D
    case 14: {sseg = 0x8600; break;} // retorna val p/ E
    case 15: {sseg = 0x8E00; break;} // retorna val p/ F
    case 16: {sseg = 0xFF00; break;} // default = tudo desligado
    default: {sseg = 0xBF00; break;} // ERRO retorna "-" (so' g ligado)
  }
  if (TIPO_DISPLAY == 0)             // ANODO COMUM sai como a tabela
    return sseg;
  else                               // CATODO inverte bits (bitwise)
    return ~sseg;
}

// FUNCAO que serializa os dados de 'ser_data' o 74HC595
void serializar(int ser_data)
{
  int stts = 15;                 // envia bit MSB 1o. = dp na placa
  do
  {
    if ((ser_data >> stts) & 1)  // se ser_data desloc >> ssts ='1'
    {
      GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);// SDATA=1
    } else {
      GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);// SDATA=0
    }
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);   // SCK=1
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET); // SCK=0
    stts--;
  } while (stts>=0);
	// depois de serializar tudo, tem que gerar RCK
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);   // RCK=1
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET); // RCK=0
}

// esta função zera os pinos ao inicializar a placa
void reset_pin_GPIOs (void){
  // garantir que pinos serial comecam com zero
  GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);  // SDATA=0
  GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);   // SCK=0
  GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);   // RCK=0
  GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);    // apaga LED PC13
  GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);  // apaga LED PC14
}

// -- (parte b) -- final das demais funções do programa
