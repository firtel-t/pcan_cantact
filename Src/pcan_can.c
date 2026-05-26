#if defined(STM32G431xx)
/* ================================================================
 * FDCAN driver for STM32G431 (MKS CANable V2.0)
 * Classic CAN mode only (no FD frames).
 * ================================================================ */
#include <stm32g4xx_hal.h>
#include <string.h>
#include <assert.h>
#include "pcan_can.h"
#include "pcan_timestamp.h"
#include "pcan_varian.h"

#define CAN_TX_FIFO_SIZE (100)

static FDCAN_HandleTypeDef g_hfdcan;

static struct
{
  uint32_t tx_msgs;
  uint32_t tx_errs;
  uint32_t tx_ovfs;
  uint32_t rx_msgs;
  uint32_t rx_errs;
  uint32_t rx_ovfs;
  can_message_t tx_fifo[CAN_TX_FIFO_SIZE];
  uint32_t tx_head;
  uint32_t tx_tail;
  void (*rx_cb)(can_message_t *);
  void (*can_err_cb)( uint8_t err, uint8_t rx_err, uint8_t tx_err );
}
can_dev = { 0 };

void pcan_can_init(void)
{
  FDCAN_FilterTypeDef filter = { 0 };

  __HAL_RCC_FDCAN_CLK_ENABLE();

  PIN_ENABLE_CLOCK( CAN_RX );
  PIN_ENABLE_CLOCK( CAN_TX );
  PIN_INIT( CAN_RX );
  PIN_INIT( CAN_TX );

  g_hfdcan.Instance = FDCAN1;
  g_hfdcan.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  g_hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  g_hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
  g_hfdcan.Init.AutoRetransmission = ENABLE;
  g_hfdcan.Init.TransmitPause = DISABLE;
  g_hfdcan.Init.ProtocolException = DISABLE;
  /* Default 500kbps @ 80MHz FDCAN clock */
  g_hfdcan.Init.NominalPrescaler = 10;
  g_hfdcan.Init.NominalSyncJumpWidth = 1;
  g_hfdcan.Init.NominalTimeSeg1 = 13;
  g_hfdcan.Init.NominalTimeSeg2 = 2;
  g_hfdcan.Init.DataPrescaler = 10;
  g_hfdcan.Init.DataSyncJumpWidth = 1;
  g_hfdcan.Init.DataTimeSeg1 = 13;
  g_hfdcan.Init.DataTimeSeg2 = 2;
  g_hfdcan.Init.StdFiltersNbr = 1;
  g_hfdcan.Init.ExtFiltersNbr = 1;
  g_hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

  if( HAL_FDCAN_Init( &g_hfdcan ) != HAL_OK )
  {
    assert( 0 );
  }

  HAL_FDCAN_ConfigGlobalFilter( &g_hfdcan,
    FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
    FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE );

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;
  HAL_FDCAN_ConfigFilter( &g_hfdcan, &filter );

  filter.IdType = FDCAN_EXTENDED_ID;
  filter.FilterIndex = 0;
  filter.FilterID1 = 0x00000000;
  filter.FilterID2 = 0x00000000;
  HAL_FDCAN_ConfigFilter( &g_hfdcan, &filter );

  HAL_FDCAN_ActivateNotification( &g_hfdcan,
    FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_FULL |
    FDCAN_IT_RX_FIFO1_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_FULL |
    FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_FIFO_EMPTY |
    FDCAN_IT_BUS_OFF | FDCAN_IT_ERROR_WARNING |
    FDCAN_IT_ERROR_PASSIVE |
    FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR,
    0xFFFFFFFF );

  HAL_NVIC_SetPriority( FDCAN1_IT0_IRQn, 0, 0 );
  HAL_NVIC_EnableIRQ( FDCAN1_IT0_IRQn );
  HAL_NVIC_SetPriority( FDCAN1_IT1_IRQn, 0, 0 );
  HAL_NVIC_EnableIRQ( FDCAN1_IT1_IRQn );
}

void pcan_can_set_bitrate( uint16_t brp, uint8_t tseg1, uint8_t tseg2, uint8_t sjw )
{
  uint32_t fdcan_clk = 80000000u;  /* PLLQ = 80MHz */
  uint32_t pcan_clk = 8000000u;    /* Original PCAN CAN clock */

  uint32_t new_brp = ((uint32_t)brp * fdcan_clk + (pcan_clk/2)) / pcan_clk;
  if( new_brp < 1 ) new_brp = 1;
  if( new_brp > 512 ) new_brp = 512;
  if( sjw < 1 ) sjw = 1;
  if( sjw > 128 ) sjw = 128;
  if( tseg1 < 1 ) tseg1 = 1;
  if( tseg2 < 1 ) tseg2 = 1;

  HAL_FDCAN_Stop( &g_hfdcan );

  g_hfdcan.Init.NominalPrescaler = new_brp;
  g_hfdcan.Init.NominalSyncJumpWidth = sjw;
  g_hfdcan.Init.NominalTimeSeg1 = tseg1;
  g_hfdcan.Init.NominalTimeSeg2 = tseg2;

  if( HAL_FDCAN_Init( &g_hfdcan ) != HAL_OK )
  {
    assert( 0 );
  }
}

void pcan_can_install_rx_callback( void (*cb)( can_message_t * ) )
{
  can_dev.rx_cb = cb;
}

void pcan_can_install_error_callback( void (*cb)( uint8_t, uint8_t, uint8_t ) )
{
  can_dev.can_err_cb = cb;
}

static int pcan_try_send_message( const can_message_t *p_msg )
{
  FDCAN_TxHeaderTypeDef tx_hdr = { 0 };

  if( p_msg->flags & CAN_FLAG_EXTID )
  {
    tx_hdr.Identifier = p_msg->id & 0x1FFFFFFF;
    tx_hdr.IdType = FDCAN_EXTENDED_ID;
  }
  else
  {
    tx_hdr.Identifier = p_msg->id & 0x7FF;
    tx_hdr.IdType = FDCAN_STANDARD_ID;
  }

  tx_hdr.TxFrameType = (p_msg->flags & CAN_FLAG_RTR) ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
  tx_hdr.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  tx_hdr.BitRateSwitch = FDCAN_BRS_OFF;
  tx_hdr.FDFormat = FDCAN_CLASSIC_CAN;
  tx_hdr.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  tx_hdr.MessageMarker = 0;

  switch( p_msg->dlc )
  {
    case 0: tx_hdr.DataLength = FDCAN_DLC_BYTES_0; break;
    case 1: tx_hdr.DataLength = FDCAN_DLC_BYTES_1; break;
    case 2: tx_hdr.DataLength = FDCAN_DLC_BYTES_2; break;
    case 3: tx_hdr.DataLength = FDCAN_DLC_BYTES_3; break;
    case 4: tx_hdr.DataLength = FDCAN_DLC_BYTES_4; break;
    case 5: tx_hdr.DataLength = FDCAN_DLC_BYTES_5; break;
    case 6: tx_hdr.DataLength = FDCAN_DLC_BYTES_6; break;
    case 7: tx_hdr.DataLength = FDCAN_DLC_BYTES_7; break;
    default: tx_hdr.DataLength = FDCAN_DLC_BYTES_8; break;
  }

  if( HAL_FDCAN_AddMessageToTxFifoQ( &g_hfdcan, &tx_hdr, (uint8_t*)p_msg->data ) != HAL_OK )
    return -1;

  return 0;
}

static void pcan_can_flush_tx( void )
{
  if( can_dev.tx_head == can_dev.tx_tail )
    return;
  can_message_t *p_msg = &can_dev.tx_fifo[can_dev.tx_tail];
  if( pcan_try_send_message( p_msg ) < 0 )
    return;
  uint32_t tail = can_dev.tx_tail + 1;
  if( tail == CAN_TX_FIFO_SIZE ) tail = 0;
  can_dev.tx_tail = tail;
}

int pcan_can_send_message( const can_message_t *p_msg )
{
  if( !p_msg ) return 0;
  uint32_t head = can_dev.tx_head + 1;
  if( head == CAN_TX_FIFO_SIZE ) head = 0;
  if( head == can_dev.tx_tail ) { ++can_dev.tx_ovfs; return -1; }
  can_dev.tx_fifo[can_dev.tx_head] = *p_msg;
  can_dev.tx_head = head;
  return 0;
}

void pcan_can_set_silent( uint8_t silent_mode )
{
  HAL_FDCAN_Stop( &g_hfdcan );
  g_hfdcan.Init.Mode = silent_mode ? FDCAN_MODE_BUS_MONITORING : FDCAN_MODE_NORMAL;
  HAL_FDCAN_Init( &g_hfdcan );
}

void pcan_can_set_loopback( uint8_t loopback )
{
  HAL_FDCAN_Stop( &g_hfdcan );
  g_hfdcan.Init.Mode = loopback ? FDCAN_MODE_INTERNAL_LOOPBACK : FDCAN_MODE_NORMAL;
  HAL_FDCAN_Init( &g_hfdcan );
}

void pcan_can_set_bus_active( uint16_t mode )
{
  if( mode ) HAL_FDCAN_Start( &g_hfdcan );
  else       HAL_FDCAN_Stop( &g_hfdcan );
}

static void pcan_can_rx_frame( uint32_t fifo )
{
  FDCAN_RxHeaderTypeDef hdr = { 0 };
  can_message_t msg = { 0 };
  uint32_t rx_location = (fifo == 0) ? FDCAN_RX_FIFO0 : FDCAN_RX_FIFO1;

  if( HAL_FDCAN_GetRxMessage( &g_hfdcan, rx_location, &hdr, msg.data ) != HAL_OK )
    return;

  if( hdr.IdType == FDCAN_STANDARD_ID ) msg.id = hdr.Identifier;
  else { msg.id = hdr.Identifier; msg.flags |= CAN_FLAG_EXTID; }
  if( hdr.RxFrameType == FDCAN_REMOTE_FRAME ) msg.flags |= CAN_FLAG_RTR;

  switch( hdr.DataLength )
  {
    case FDCAN_DLC_BYTES_0: msg.dlc = 0; break;
    case FDCAN_DLC_BYTES_1: msg.dlc = 1; break;
    case FDCAN_DLC_BYTES_2: msg.dlc = 2; break;
    case FDCAN_DLC_BYTES_3: msg.dlc = 3; break;
    case FDCAN_DLC_BYTES_4: msg.dlc = 4; break;
    case FDCAN_DLC_BYTES_5: msg.dlc = 5; break;
    case FDCAN_DLC_BYTES_6: msg.dlc = 6; break;
    case FDCAN_DLC_BYTES_7: msg.dlc = 7; break;
    default: msg.dlc = 8; break;
  }

  msg.timestamp = pcan_timestamp_ticks();
  if( can_dev.rx_cb ) can_dev.rx_cb( &msg );
  ++can_dev.rx_msgs;
}

void pcan_can_poll(void)
{
  while( HAL_FDCAN_GetRxFifoFillLevel( &g_hfdcan, FDCAN_RX_FIFO0 ) > 0 )
    pcan_can_rx_frame( 0 );
  while( HAL_FDCAN_GetRxFifoFillLevel( &g_hfdcan, FDCAN_RX_FIFO1 ) > 0 )
    pcan_can_rx_frame( 1 );
  pcan_can_flush_tx();
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
  (void)hfdcan;
  if( RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) pcan_can_rx_frame( 0 );
  if( RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL ) ++can_dev.rx_ovfs;
}

void HAL_FDCAN_RxFifo1Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs )
{
  (void)hfdcan;
  if( RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE ) pcan_can_rx_frame( 1 );
  if( RxFifo1ITs & FDCAN_IT_RX_FIFO1_FULL ) ++can_dev.rx_ovfs;
}

void HAL_FDCAN_TxFifoEmptyCallback( FDCAN_HandleTypeDef *hfdcan )
{
  (void)hfdcan;
  ++can_dev.tx_msgs;
}

void HAL_FDCAN_ErrorStatusCallback( FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs )
{
  (void)hfdcan;
  uint8_t can_err = 0;
  FDCAN_ErrorCountersTypeDef err_cnt;
  HAL_FDCAN_GetErrorCounters( &g_hfdcan, &err_cnt );

  if( ErrorStatusITs & FDCAN_IT_BUS_OFF ) can_err |= CAN_ERROR_FLAG_BUSOFF;
  if( ErrorStatusITs & (FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR) )
  { ++can_dev.tx_errs; can_err |= CAN_ERROR_FLAG_TX_ERR; }

  if( can_dev.can_err_cb && can_err )
    can_dev.can_err_cb( can_err, err_cnt.TxErrorCnt, err_cnt.RxErrorCnt );
}

void FDCAN1_IT0_IRQHandler( void ) { HAL_FDCAN_IRQHandler( &g_hfdcan ); }
void FDCAN1_IT1_IRQHandler( void ) { HAL_FDCAN_IRQHandler( &g_hfdcan ); }

#else
/* ================================================================
 * bxCAN driver for STM32F042
 * ================================================================ */
#include <stm32f0xx_hal.h>
#include <string.h>
#include <assert.h>
#include "pcan_can.h"
#include "pcan_timestamp.h"
#include "pcan_varian.h"

#define CAN_TX_FIFO_SIZE (100)
static CAN_HandleTypeDef g_hcan = { .Instance = CAN };
#define INTERNAL_CAN_IT_FLAGS (CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_BUSOFF | CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR)

static struct
{
  uint32_t tx_msgs; uint32_t tx_errs; uint32_t tx_ovfs;
  uint32_t rx_msgs; uint32_t rx_errs; uint32_t rx_ovfs;
  can_message_t tx_fifo[CAN_TX_FIFO_SIZE];
  uint32_t tx_head; uint32_t tx_tail;
  void (*rx_cb)(can_message_t *);
  void (*can_err_cb)( uint8_t err, uint8_t rx_err, uint8_t tx_err );
} can_dev = { 0 };

void pcan_can_init(void)
{
  CAN_FilterTypeDef filter = { 0 };
  __HAL_RCC_CAN1_CLK_ENABLE();
  PIN_ENABLE_CLOCK( CAN_RX ); PIN_ENABLE_CLOCK( CAN_TX );
  PIN_INIT( CAN_RX ); PIN_INIT( CAN_TX );
  HAL_CAN_DeInit( &g_hcan );
  g_hcan.Instance = CAN;
  g_hcan.Init.Prescaler = 16;
  g_hcan.Init.Mode = CAN_MODE_NORMAL;
  g_hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  g_hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
  g_hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  g_hcan.Init.TimeTriggeredMode = DISABLE;
  g_hcan.Init.AutoBusOff = ENABLE;
  g_hcan.Init.AutoWakeUp = DISABLE;
  g_hcan.Init.AutoRetransmission = ENABLE;
  g_hcan.Init.ReceiveFifoLocked = DISABLE;
  g_hcan.Init.TransmitFifoPriority = ENABLE;
  if( HAL_CAN_Init( &g_hcan ) != HAL_OK ) { assert( 0 ); }
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterActivation = ENABLE;
  filter.FilterBank = 0;
  filter.SlaveStartFilterBank = 0;
  if( HAL_CAN_ConfigFilter( &g_hcan, &filter ) != HAL_OK ) { assert( 0 ); }
  if( HAL_CAN_ActivateNotification( &g_hcan, INTERNAL_CAN_IT_FLAGS ) != HAL_OK ) { assert( 0 ); }
}

void pcan_can_set_bitrate( uint16_t brp, uint8_t tseg1, uint8_t tseg2, uint8_t sjw )
{
  static const uint32_t sjw_table[] = { CAN_SJW_1TQ, CAN_SJW_2TQ, CAN_SJW_3TQ, CAN_SJW_4TQ };
  static const uint32_t tseg1_table[] = { CAN_BS1_1TQ, CAN_BS1_2TQ, CAN_BS1_3TQ, CAN_BS1_4TQ, CAN_BS1_5TQ, CAN_BS1_6TQ, CAN_BS1_7TQ, CAN_BS1_8TQ, CAN_BS1_9TQ, CAN_BS1_10TQ, CAN_BS1_11TQ, CAN_BS1_12TQ, CAN_BS1_13TQ, CAN_BS1_14TQ, CAN_BS1_15TQ, CAN_BS1_16TQ };
  static const uint32_t tseg2_table[] = { CAN_BS2_1TQ, CAN_BS2_2TQ, CAN_BS2_3TQ, CAN_BS2_4TQ, CAN_BS2_5TQ, CAN_BS2_6TQ, CAN_BS2_7TQ, CAN_BS2_8TQ };
  if( sjw > 4 ) sjw = 4;
  if( tseg1 > 16 ) tseg1 = 16;
  if( tseg2 > 8 ) tseg2 = 8;
  g_hcan.Init.Prescaler = brp * 6;
  g_hcan.Init.SyncJumpWidth = sjw_table[sjw - 1];
  g_hcan.Init.TimeSeg1 = tseg1_table[tseg1 - 1];
  g_hcan.Init.TimeSeg2 = tseg2_table[tseg2 - 1];
  if( HAL_CAN_Init( &g_hcan ) != HAL_OK ) { assert( 0 ); }
}

void pcan_can_install_rx_callback( void (*cb)( can_message_t * ) ) { can_dev.rx_cb = cb; }
void pcan_can_install_error_callback( void (*cb)( uint8_t, uint8_t, uint8_t ) ) { can_dev.can_err_cb = cb; }

static int pcan_try_send_message( const can_message_t *p_msg )
{
  CAN_TxHeaderTypeDef msg = { .TransmitGlobalTime = DISABLE };
  uint32_t txMailbox = 0;
  if( p_msg->flags & CAN_FLAG_EXTID ) { msg.ExtId = p_msg->id & 0x1FFFFFFF; msg.IDE = CAN_ID_EXT; }
  else { msg.StdId = p_msg->id & 0x7FF; msg.IDE = CAN_ID_STD; }
  msg.DLC = p_msg->dlc;
  msg.RTR = (p_msg->flags & CAN_FLAG_RTR)?CAN_RTR_REMOTE:CAN_RTR_DATA;
  if( HAL_CAN_AddTxMessage( &g_hcan, &msg, (void*)p_msg->data, &txMailbox ) != HAL_OK ) return -1;
  return txMailbox;
}

static void pcan_can_flush_tx( void )
{
  if( can_dev.tx_head == can_dev.tx_tail ) return;
  can_message_t *p_msg = &can_dev.tx_fifo[can_dev.tx_tail];
  if( pcan_try_send_message( p_msg ) < 0 ) return;
  uint32_t tail = can_dev.tx_tail+1;
  if( tail == CAN_TX_FIFO_SIZE ) tail = 0;
  can_dev.tx_tail = tail;
}

int pcan_can_send_message( const can_message_t *p_msg )
{
  if( !p_msg ) return 0;
  uint32_t head = can_dev.tx_head+1;
  if( head == CAN_TX_FIFO_SIZE ) head = 0;
  if( head == can_dev.tx_tail ) { ++can_dev.tx_ovfs; return -1; }
  can_dev.tx_fifo[can_dev.tx_head] = *p_msg;
  can_dev.tx_head = head;
  return 0;
}

void pcan_can_set_silent( uint8_t silent_mode )
{
  g_hcan.Init.Mode = silent_mode ? CAN_MODE_SILENT: CAN_MODE_NORMAL;
  if( HAL_CAN_Init( &g_hcan ) != HAL_OK ) { assert( 0 ); }
}

void pcan_can_set_loopback( uint8_t loopback )
{
  g_hcan.Init.Mode = loopback ? CAN_MODE_LOOPBACK: CAN_MODE_NORMAL;
  if( HAL_CAN_Init( &g_hcan ) != HAL_OK ) { assert( 0 ); }
}

void pcan_can_set_bus_active( uint16_t mode )
{
  if( mode ) { HAL_CAN_Start( &g_hcan ); HAL_CAN_AbortTxRequest( &g_hcan, CAN_TX_MAILBOX0|CAN_TX_MAILBOX1|CAN_TX_MAILBOX2 ); }
  else { HAL_CAN_AbortTxRequest( &g_hcan, CAN_TX_MAILBOX0|CAN_TX_MAILBOX1|CAN_TX_MAILBOX2 ); HAL_CAN_Stop( &g_hcan ); }
}

static void pcan_can_rx_frame( CAN_HandleTypeDef *hcan, uint32_t fifo )
{
  CAN_RxHeaderTypeDef hdr = { 0 };
  can_message_t msg = { 0 };
  if( HAL_CAN_GetRxMessage( hcan, fifo, &hdr, msg.data ) != HAL_OK ) return;
  if( hdr.IDE == CAN_ID_STD ) msg.id = hdr.StdId;
  else { msg.id = hdr.ExtId; msg.flags |= CAN_FLAG_EXTID; }
  if( hdr.RTR == CAN_RTR_REMOTE ) msg.flags |= CAN_FLAG_RTR;
  msg.dlc = hdr.DLC;
  msg.timestamp = pcan_timestamp_ticks();
  if( can_dev.rx_cb ) can_dev.rx_cb( &msg );
  ++can_dev.rx_msgs;
}

void pcan_can_poll(void)
{
  HAL_CAN_IRQHandler( &g_hcan );
  pcan_can_flush_tx();
}

void HAL_CAN_RxFifo0MsgPendingCallback( CAN_HandleTypeDef *hcan ) { pcan_can_rx_frame( hcan, CAN_RX_FIFO0 ); }
void HAL_CAN_RxFifo1MsgPendingCallback( CAN_HandleTypeDef *hcan ) { pcan_can_rx_frame( hcan, CAN_RX_FIFO1 ); }
void HAL_CAN_TxMailbox0CompleteCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); ++can_dev.tx_msgs; }
void HAL_CAN_TxMailbox1CompleteCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); ++can_dev.tx_msgs; }
void HAL_CAN_TxMailbox2CompleteCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); ++can_dev.tx_msgs; }
void HAL_CAN_RxFifo0FullCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); ++can_dev.rx_ovfs; }
void HAL_CAN_RxFifo1FullCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); ++can_dev.rx_ovfs; }
void HAL_CAN_SleepCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); }
void HAL_CAN_WakeUpFromRxMsgCallback( CAN_HandleTypeDef *hcan ) { UNUSED(hcan); }

void HAL_CAN_ErrorCallback( CAN_HandleTypeDef *hcan )
{
  uint32_t err = HAL_CAN_GetError( hcan );
  uint8_t can_err = 0;
  if( err & (HAL_CAN_ERROR_TX_TERR0|HAL_CAN_ERROR_TX_TERR1|HAL_CAN_ERROR_TX_TERR2) ) { ++can_dev.tx_errs; can_err |= CAN_ERROR_FLAG_TX_ERR; }
  if( err & HAL_CAN_ERROR_BOF ) can_err |= CAN_ERROR_FLAG_BUSOFF;
  if( err & (HAL_CAN_ERROR_RX_FOV0|HAL_CAN_ERROR_RX_FOV1) ) can_err |= CAN_ERROR_FLAG_RX_OVF;
  if( can_dev.can_err_cb && can_err ) can_dev.can_err_cb( can_err, can_dev.tx_errs & 0xFF, can_dev.rx_errs );
  HAL_CAN_ResetError( hcan );
}

#endif /* STM32G431xx */
