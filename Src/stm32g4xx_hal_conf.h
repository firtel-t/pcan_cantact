/*
 * stm32g4xx_hal_conf.h - HAL configuration for STM32G431 PCAN project
 */
#pragma once

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_FDCAN_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_PWR_EX_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_RCC_EX_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED

/* ########################## HSE/HSI Values ########################### */
#if !defined (HSE_VALUE)
  #define HSE_VALUE    8000000U  /*!< Value of the External oscillator in Hz */
#endif

#if !defined (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U   /*!< Time out for HSE start up, in ms */
#endif

#if !defined (HSI_VALUE)
  #define HSI_VALUE    16000000U /*!< Value of the Internal oscillator in Hz */
#endif

#if !defined (HSI48_VALUE)
  #define HSI48_VALUE  48000000U /*!< Value of the Internal High Speed oscillator for USB in Hz */
#endif

#if !defined (LSI_VALUE)
  #define LSI_VALUE    32000U    /*!< LSI Typical Value in Hz */
#endif

#if !defined (LSE_VALUE)
  #define LSE_VALUE    32768U    /*!< Value of the External Low Speed oscillator in Hz */
#endif

#if !defined (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U  /*!< Time out for LSE start up, in ms */
#endif

#if !defined (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    12288000U /*!< Value of the External clock in Hz */
#endif

/* ########################### System Configuration ##################### */
#define  VDD_VALUE                    3300U /*!< Value of VDD in mv */
#define  TICK_INT_PRIORITY            0U    /*!< tick interrupt priority */
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  INSTRUCTION_CACHE_ENABLE     1U
#define  DATA_CACHE_ENABLE            1U

/* ########################## Assert Selection ########################## */
/* #define USE_FULL_ASSERT    1U */

/* ################## SPI peripheral configuration ###################### */
/* CRC FEATURE: Use to activate CRC feature inside HAL SPI Driver */
/* #define USE_SPI_CRC                     1U */

/* Includes ------------------------------------------------------------------*/
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32g4xx_hal_rcc.h"
  #include "stm32g4xx_hal_rcc_ex.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32g4xx_hal_gpio.h"
  #include "stm32g4xx_hal_gpio_ex.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32g4xx_hal_dma.h"
  #include "stm32g4xx_hal_dma_ex.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32g4xx_hal_cortex.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32g4xx_hal_exti.h"
#endif

#ifdef HAL_FDCAN_MODULE_ENABLED
  #include "stm32g4xx_hal_fdcan.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32g4xx_hal_flash.h"
  #include "stm32g4xx_hal_flash_ex.h"
  #include "stm32g4xx_hal_flash_ramfunc.h"
#endif

#ifdef HAL_PCD_MODULE_ENABLED
  #include "stm32g4xx_hal_pcd.h"
  #include "stm32g4xx_hal_pcd_ex.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32g4xx_hal_pwr.h"
  #include "stm32g4xx_hal_pwr_ex.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32g4xx_hal_tim.h"
  #include "stm32g4xx_hal_tim_ex.h"
#endif

#ifdef USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif
