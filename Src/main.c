#if defined(STM32G431xx)
/* ================================================================
 * STM32G431 (MKS CANable V2.0) main
 * System clock: 160MHz from HSI via PLL
 * USB clock: 48MHz from HSI48 + CRS
 * FDCAN clock: 80MHz from PLLQ
 * ================================================================ */
#include "stm32g4xx_hal.h"
#include "pcan_timestamp.h"
#include "pcan_led.h"
#include "pcan_protocol.h"
#include "pcan_usb.h"
#include "pcan_varian.h"

void HAL_MspInit( void )
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
}

void SystemClock_Config( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* Configure the main internal regulator output voltage */
  HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1_BOOST );

  /* Enable HSI and HSI48 oscillators, configure PLL from HSI */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;   /* 16MHz / 4 = 4MHz */
  RCC_OscInitStruct.PLL.PLLN = 80;               /* 4MHz * 80 = 320MHz VCO */
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;   /* 160MHz */
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;   /* 320/4 = 80MHz for FDCAN */
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;   /* 320/2 = 160MHz SYSCLK */

  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
  {
    while(1);
  }

  /* Select PLL as system clock source and configure bus clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK )
  {
    while(1);
  }

  /* USB clock from HSI48 */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
  {
    while(1);
  }

  /* Enable CRS for HSI48 calibration from USB SOF */
  __HAL_RCC_CRS_CLK_ENABLE();

  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE( 48000000, 1000 );
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig( &RCC_CRSInitStruct );

  /* FDCAN clock from PLLQ (80MHz) */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
  PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;

  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
  {
    while(1);
  }
}

#else
/* ================================================================
 * STM32F042 (CANtact / CANable / Entree / Ollie) main
 * ================================================================ */
#include "stm32f0xx_hal.h"
#include "pcan_timestamp.h"
#include "pcan_led.h"
#include "pcan_protocol.h"
#include "pcan_usb.h"
#include "pcan_varian.h"

void HAL_MspInit( void )
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

#if ( HSE_VALUE != 0 )
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* enable HSE */
   __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
  while( __HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET );

  /* enable PLL */
  __HAL_RCC_PLL_DISABLE();
  while( __HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)  != RESET );
#if ( HSE_VALUE == 16000000 )
  __HAL_RCC_PLL_CONFIG( RCC_PLLSOURCE_HSE, RCC_PREDIV_DIV1, RCC_PLL_MUL3 );
#elif ( HSE_VALUE == 8000000 )
  __HAL_RCC_PLL_CONFIG( RCC_PLLSOURCE_HSE, RCC_PREDIV_DIV1, RCC_PLL_MUL6 );
#endif
  __HAL_RCC_PLL_ENABLE();
  while( __HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)  == RESET );

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );

  __HAL_RCC_USB_CONFIG( RCC_USBCLKSOURCE_PLL );
}
#else
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  __HAL_RCC_HSI48_ENABLE();
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY) == RESET );

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
    |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  __HAL_RCC_USB_CONFIG( RCC_USBCLKSOURCE_HSI48 );

  /* CRS */
  __HAL_RCC_CRS_CLK_ENABLE();
  
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE( 48000000, 1000 );
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;

  HAL_RCCEx_CRSConfig( &RCC_CRSInitStruct );
}
#endif
#endif /* STM32G431xx */

void SysTick_Handler( void )
{
  HAL_IncTick();
}

int main( void )
{
  HAL_Init();
  HAL_IncTick();

  SystemClock_Config();

  pcan_variant_io_init();

  pcan_usb_init();
  pcan_led_init();
  pcan_timestamp_init();
  pcan_protocol_init();

  pcan_led_set_mode( LED_CH0_RX, LED_MODE_BLINK_SLOW, 0 );
  pcan_led_set_mode( LED_CH0_TX, LED_MODE_BLINK_SLOW, 0 );

  for(;;)
  {
    pcan_usb_poll();
    pcan_led_poll();
    pcan_protocol_poll();
  }
}
