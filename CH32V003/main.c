/* 
 *  KETACLOCK clone
 *  1 digit segment LCD - CH32V003F4P6
 */

/*
   1 - PD4 - DIG1      20 - PD3 - DIG0
   2 - *TXD            19 - PD2 - IR in
   3 - *RXD            18 - PD1 - *SWIO
   4 - PD7 - *NRST     17 - PC7 - SEG F
   5 - PA1 - DIG2      16 - PC6 - SEG A
   6 - PA2 - SEG COM   15 - PC5 - SEG B
   7 - VSS - GND       14 - PC4 - SEG G
   8 - PD0 - LED       13 - PC3 - SEG E
   9 - VDD - 3.3V      12 - PC2 - SEG D
  10 - PC0 - SEG DP    11 - PC1 - SEG C
*/

/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : n24bass@gmail.com
 * Version            : V1.0.0
 * Date               : 2023/08/08
 * Description        : Main program body.
 *******************************************************************************/

#include "debug.h"

uint32_t value;

// PD0 as LED
#define LED_ON()    GPIO_SetBits(GPIOD, GPIO_Pin_0)
#define LED_OFF()   GPIO_ResetBits(GPIOD, GPIO_Pin_0)

void IOPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE); // GP
  // PA1 as input (digit selecter)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // PD3,4 as input (digit selecter)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  // PA2 as out (COM)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // PC* as output (segment LCD)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // PD0 as output (LED)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

}

/*********************************************************************  
 * @fn      Input_Capture_Init
 *
 * @brief   Initializes TIM1 input capture for IR receiver
 *
 * @param   arr - the period value.
 *          psc - the prescaler value.
 *          ccp - the pulse value.
 *
 * @return  none
 */
void Input_Capture_Init(u16 arr, u16 psc)
{
  // GPIOD2 for IR receiver
  
  GPIO_InitTypeDef        GPIO_InitStructure = {0};
  TIM_ICInitTypeDef       TIM_ICInitStructure = {0};
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
  NVIC_InitTypeDef        NVIC_InitStructure = {0};

  // clock for GPIOD2 and T1CH1 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1, ENABLE); // GPIO-D2 -> T1CH1

  // GPIOD2(T1CH1) as input
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // = T1CH1
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOD, GPIO_Pin_2); //

  // TimeBase unit
  TIM_TimeBaseInitStructure.TIM_Period = arr;
  TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

  // PWM input
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x00;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_PWMIConfig(TIM1, &TIM_ICInitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE); // TIM_IT_CC1 |

  TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);
  TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);
  TIM_Cmd(TIM1, ENABLE);
}

// get digit position for display 0..6 (PD0,PD1,PD7)
int getDigit() {
  int d = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) ? 0: 4) // PA1
    + (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4) ? 0: 2) // PD1
    + (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) ? 0: 1); // PD0
  return d % 6;
}

uint8_t reverse(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void val2digits(uint32_t val, uint8_t digits[]) {
  digits[5] = reverse(val >> 16) & 0xff; // hour
  digits[3] = reverse(val >> 24) & 0xff; // min
  digits[1] = reverse(val >> 8) & 0xff; // sec

  printf("%02d:%02d:%02d\r\n", digits[5], digits[3], digits[1]); // debug print

  for (int i = 0; i < 6; i++) {
    if (i % 2) {
      digits[i] /= 10;
    } else {
      digits[i] = digits[i + 1] % 10;
    }
  }
}

// dispLCD(v:0..9, dp:0,1)
#define SEGMENTS 8

// 7 SEGMENT LCD pin assign
// const
uint16_t segments[SEGMENTS + 1] = {
  GPIO_Pin_0, // DP - PC0
  GPIO_Pin_4, // G  - PC4
  GPIO_Pin_7, // F  - PC7
  GPIO_Pin_3, // E  - PC3
  GPIO_Pin_2, // D  - PC2
  GPIO_Pin_1, // C  - PC1
  GPIO_Pin_5, // B  - PC5
  GPIO_Pin_6, // A  - PC6
  //
  GPIO_Pin_2  // COM - PA2
};
  
/*
 * segment pattern
 * bit7:A..bit1:G,bit0:DP
 */
// const
uint8_t digitSP[10] = {
  0xfc, // 0
  0x60, // 1
  0xDa, // 2
  0xf2, // 3
  0x66, // 4
  0xb6, // 5
  0xbe, // 6
  0xe0, // 7
  0xfe, // 8
  0xf6, // 9
  // 0x00, // none
};

void dispLCD(int v, int dp) {
  static uint8_t toggle = 0;

  // segment pattern
  uint8_t c = digitSP[v]; // base segment pattern
  if (dp) c |= 0x01; // DP on
  if (toggle) c = ~c; // reverse for AC drive

  for (int i = 0; i < SEGMENTS; i++) {
    GPIO_WriteBit(GPIOC, segments[i], (c & 0x01)? Bit_SET: Bit_RESET); // segments: GPIO-PC*
    c >>= 1;
  }
  // GPIO_WriteBit(GPIOA, segments[SEGMENTS], toggle? Bit_SET: Bit_RESET); // common: GPIO-PA*
  GPIO_WriteBit(GPIOA, GPIO_Pin_2, toggle? Bit_SET: Bit_RESET); // common: GPIO-PA2

  toggle = !toggle;
}

// uint16_t get_tick(uint16_t t)
// {
//   return (uint16_t)(t / 56.2 + 0.5);
// }

#define TICKS(X) (uint16_t)((X) / 56.2 + 0.5)

void TIM1_CC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      TIM1_CC_IRQHandler
 *
 * @brief   This function handles TIM1  Capture Compare Interrupt exception.
 *
 * @return  none
 */
void TIM1_CC_IRQHandler(void)
{
  static int running = 0;
  static int count;
  static uint32_t val;
  
  if (TIM_GetITStatus( TIM1, TIM_IT_CC2 ) != SET) return;

  uint16_t t1 = TICKS(TIM_GetCapture1(TIM1)); // mark ticks
  uint16_t t2 = TICKS(TIM_GetCapture2(TIM1)); // blank ticks
  TIM_SetCounter(TIM1, 0); // clear count

  if (!running) {
    // wait for leader
    if (t1 == 16 && t2 == 8) {
      // NEC Leader detected
      count = 0;
      val = 0;
      running = 1;
    }
  } else {
    // wait for data bits
    if (t1 == 1 && (t2 == 1 || t2 == 3)) {
      // valid data bit (NEC)
      val <<= 1;
      if (t2 == 3) val |= 1;
      count++;
      if (count == 32) {
        // complete
        value = val; // set global
        running = 0;
      }
    } else {
      // invalid data bit
      running = 0;
    }
  }
  running? LED_ON(): LED_OFF();
}

void incrDigits(uint8_t d[], int i)
{
  int hms;
  uint32_t t = (d[0] + 10*d[1]) + 60 * (d[2] + 10*d[3]) + 60 * 60 * (d[4] + 10*d[5]) + i;
  uint32_t s = 60 * 60;

  for (int i = 2; i >= 0; i--) {
    hms = t / s;
    t -= s * hms;
    if (i == 2) if (hms == 24) hms = 0;
    d[2 * i + 1] = hms / 10;
    d[2 * i] = hms % 10;
    s /= 60;
  }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
  uint8_t digits[6];
  uint32_t pvalue = 0;

  value = pvalue;

  Delay_Init(); // initialize delay function
  USART_Printf_Init(115200); // Initializes the USARTx peripheral. (PD5:TXD,PD6:RXD)
  
  printf("SystemClk:%d\r\n", SystemCoreClock);

  IOPinInit(); // local GPIO
  LED_OFF();

  value = 0; // init global
  int d = getDigit(); // digit for display
  printf("digit:%d\r\n\r\n", d);
  // d = 0;
  // printf("AFIO:%x\r\n", AFIO->PCFR1 & 0xffff);

  Input_Capture_Init(0xFFFF, 480 - 1); // capture IR receiver - 10us

  int i = 0;
  while(1) {
    if (value != pvalue) {
      // new value
      val2digits(pvalue = value, digits);
      // printf("* %d\r\n", i);
      i = 0;
      pvalue = value;
    }
    dispLCD((int)digits[d], (i < 30)? 0: 1);
    if (i++ > 60) {
      incrDigits(digits, 1);
      i = 0;
      // printf("*\r\n");
    }
    Delay_Ms(1000 / 60);
  }
}

//
