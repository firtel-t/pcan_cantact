#include <assert.h>
#if defined(STM32G431xx)
#include "stm32g4xx.h"
#include "stm32g4xx_hal.h"
#else
#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"
#endif
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_conf.h"

PCD_HandleTypeDef hpcd_USB_FS;

static USBD_StatusTypeDef USBD_Get_USB_Status( HAL_StatusTypeDef hal_status );
extern void SystemClock_Config( void );

void HAL_PCD_MspInit( PCD_HandleTypeDef* pcdHandle )
{
  if( pcdHandle->Instance == USB )
  {
    __HAL_RCC_USB_CLK_ENABLE();

#if defined(STM32G431xx)
    /* Configure USB DM/DP pins (PA11/PA12) - analog mode for USB */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    HAL_NVIC_SetPriority( USB_LP_IRQn, 1, 0 );
    HAL_NVIC_EnableIRQ( USB_LP_IRQn );
#endif
  }
}

void HAL_PCD_MspDeInit( PCD_HandleTypeDef* pcdHandle )
{
  if( pcdHandle->Instance == USB )
  {
    __HAL_RCC_USB_CLK_DISABLE();
#if defined(STM32G431xx)
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_11 | GPIO_PIN_12 );
    HAL_NVIC_DisableIRQ( USB_LP_IRQn );
#else
    HAL_NVIC_DisableIRQ( USB_IRQn );
#endif
  }
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
  if( hpcd->Init.speed != PCD_SPEED_FULL ) { assert(0); }
  USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);
  USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Suspend( (USBD_HandleTypeDef*)hpcd->pData );
  if (hpcd->Init.low_power_enable)
  {
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  if (hpcd->Init.low_power_enable)
  {
    SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    SystemClock_Config();
  }
  USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  hpcd_USB_FS.pData = pdev;
  pdev->pData = &hpcd_USB_FS;

  hpcd_USB_FS.Instance = USB;
#if defined(STM32G431xx)
  hpcd_USB_FS.Init.dev_endpoints = 8;
#else
  hpcd_USB_FS.Init.dev_endpoints = 6;
#endif
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;

  if( HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK )
  {
    assert( 0 );
  }

#if defined(STM32G431xx)
  HAL_PCDEx_PMAConfig( pdev->pData, 0x00, PCD_SNG_BUF, 0x18 + (0 * 64) );
  HAL_PCDEx_PMAConfig( pdev->pData, 0x80, PCD_SNG_BUF, 0x18 + (1 * 64) );
  HAL_PCDEx_PMAConfig( pdev->pData, 0x01, PCD_SNG_BUF, 0x18 + (2 * 64) );
  HAL_PCDEx_PMAConfig( pdev->pData, 0x81, PCD_SNG_BUF, 0x18 + (3 * 64) );
  HAL_PCDEx_PMAConfig( pdev->pData, 0x02, PCD_SNG_BUF, 0x18 + (4 * 64) );
  HAL_PCDEx_PMAConfig( pdev->pData, 0x82, PCD_SNG_BUF, 0x18 + (5 * 64) );
#else
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x00, PCD_SNG_BUF, 0x18+(0*USB_FS_MAX_PACKET_SIZE));
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x80, PCD_SNG_BUF, 0x18+(1*USB_FS_MAX_PACKET_SIZE));
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x01, PCD_SNG_BUF, 0x18+(2*USB_FS_MAX_PACKET_SIZE));
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x81, PCD_SNG_BUF, 0x18+(3*USB_FS_MAX_PACKET_SIZE));
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x02, PCD_SNG_BUF, 0x18+(4*USB_FS_MAX_PACKET_SIZE));
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData, 0x82, PCD_SNG_BUF, 0x18+(5*USB_FS_MAX_PACKET_SIZE));
#endif

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) { return USBD_Get_USB_Status(HAL_PCD_DeInit(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) { return USBD_Get_USB_Status(HAL_PCD_Start(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) { return USBD_Get_USB_Status(HAL_PCD_Stop(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps) { return USBD_Get_USB_Status(HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type)); }
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) { return USBD_Get_USB_Status(HAL_PCD_EP_Close(pdev->pData, ep_addr)); }
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) { return USBD_Get_USB_Status(HAL_PCD_EP_Flush(pdev->pData, ep_addr)); }
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) { return USBD_Get_USB_Status(HAL_PCD_EP_SetStall(pdev->pData, ep_addr)); }
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) { return USBD_Get_USB_Status(HAL_PCD_EP_ClrStall(pdev->pData, ep_addr)); }

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;
  if((ep_addr & 0x80) == 0x80) return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  else return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr) { return USBD_Get_USB_Status(HAL_PCD_SetAddress(pdev->pData, dev_addr)); }
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) { return USBD_Get_USB_Status(HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size)); }
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) { return USBD_Get_USB_Status(HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size)); }
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr) { return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr); }
void USBD_LL_Delay(uint32_t Delay) { HAL_Delay(Delay); }
void *USBD_static_malloc(uint32_t size) { (void)size; return 0; }
void USBD_static_free(void *p) { (void)p; }

USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
  switch(hal_status)
  {
    case HAL_OK:      return USBD_OK;
    case HAL_BUSY:    return USBD_BUSY;
    default:          return USBD_FAIL;
  }
}

/* USB IRQ Handler */
#if defined(STM32G431xx)
void USB_LP_IRQHandler( void ) { HAL_PCD_IRQHandler( &hpcd_USB_FS ); }
#endif
