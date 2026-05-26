#include <assert.h>
#if defined(STM32G431xx)
#include <stm32g4xx_hal.h>
#else
#include <stm32f0xx_hal.h>
#endif

void pcan_timestamp_init( void )
{
  __HAL_RCC_TIM3_CLK_ENABLE();

#if defined(STM32G431xx)
  /*
   * TIM3 clock = 160MHz (APB1 timer clock)
   * Target tick period: 42.666us
   * Prescaler = 160MHz * 42.666us = 6826.6 => use 6827 (PSC = 6826)
   * Actual tick = 6827 / 160MHz = 42.669us (close enough)
   */
  TIM3->PSC = (6827u - 1u);
#else
  /*
   * TIM3 clock = 48MHz (APB1 bus)
   * Prescaler = 48MHz * 42.666us = 2048
   * PSC = 2047
   */
  TIM3->PSC = (2048 - 1);
#endif

  /* set clock division to zero */
  TIM3->CR1 &= (uint16_t)(~TIM_CR1_CKD);
  TIM3->CR1 |= TIM_CLOCKDIVISION_DIV1;
  /* Auto-reload at max (free-running 16-bit counter) */
  TIM3->ARR = 0xFFFF;
  /* Generate update event to load prescaler */
  TIM3->EGR = TIM_EGR_UG;
  /* Enable timer */
  TIM3->CR1 |= TIM_CR1_CEN;
}

uint16_t pcan_timestamp_millis( void )
{
  return (HAL_GetTick() & 0xFFFF);
}

uint16_t pcan_timestamp_ticks( void )
{
  /* 1 pcan tick => ~42.666 us */
  return TIM3->CNT;
}
