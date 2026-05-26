/*
 * system_stm32g4xx.c - System initialization for STM32G431
 *
 * Minimal system init. Clock configuration is done in main.c via HAL.
 */
#include "stm32g4xx.h"

/* Vector table offset (default: 0) */
#if !defined(VECT_TAB_OFFSET)
  #define VECT_TAB_OFFSET  0x00U
#endif

uint32_t SystemCoreClock = 16000000U; /* Will be updated by HAL_RCC_ClockConfig */

const uint8_t AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                    1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8]  = {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};

void SystemInit( void )
{
  /* FPU settings */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << (10*2)) | (3UL << (11*2)));  /* set CP10 and CP11 Full Access */
#endif

  /* Configure the Vector Table location */
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
}

void SystemCoreClockUpdate( void )
{
  uint32_t tmp, pllvco, pllr, pllsource, pllm;

  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch( tmp )
  {
    case 0x04U:  /* HSI */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x08U:  /* HSE */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x0CU:  /* PLL */
      pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC);
      pllm = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos) + 1U;

      if( pllsource == 0x02U ) /* HSI */
      {
        pllvco = (HSI_VALUE / pllm);
      }
      else if( pllsource == 0x03U ) /* HSE */
      {
        pllvco = (HSE_VALUE / pllm);
      }
      else
      {
        pllvco = 0U;
      }

      pllvco = pllvco * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
      pllr = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos) + 1U) * 2U;
      SystemCoreClock = pllvco / pllr;
      break;
    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }

  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];
  SystemCoreClock >>= tmp;
}
