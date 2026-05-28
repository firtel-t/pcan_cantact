#pragma once

#include "io_macro.h"

#if (defined CANABLE) || (defined ENTREE) || (defined CANTACT_8 ) || (defined CANTACT_16 )
#define IOPIN_TX    B, 1, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define IOPIN_RX    B, 0, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define LED_ON      PIN_HI
#define LED_OFF     PIN_LOW

#define CAN_RX      B, 8, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN
#define CAN_TX      B, 9, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN

#define pcan_variant_io_init()
#elif (defined OLLIE)
#define CAN_RX      B, 8, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN
#define CAN_TX      B, 9, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN

#define OUTPUT_EN_5V  C, 13, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define OUTPUT_EN_3V3 C, 14, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define OUTPUT_EN_1V8 C, 15, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define LED_5V        A, 5, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define LED_3V3       A, 6, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define LED_1V8       A, 7, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define SW1           A, 2, MODE_INPUT, PULLUP, SPEED_FREQ_MEDIUM, NOAF
#define VS_5V         B, 6, MODE_INPUT, PULLUP, SPEED_FREQ_MEDIUM, NOAF
#define VS_3V3        B, 5, MODE_INPUT, PULLUP, SPEED_FREQ_MEDIUM, NOAF
#define VS_1V8        B, 4, MODE_INPUT, PULLUP, SPEED_FREQ_MEDIUM, NOAF

#define pcan_variant_io_init() do{\
PIN_ENABLE_CLOCK( OUTPUT_EN_5V  );\
PIN_ENABLE_CLOCK( OUTPUT_EN_3V3 );\
PIN_ENABLE_CLOCK( OUTPUT_EN_1V8 );\
PIN_ENABLE_CLOCK( LED_5V );\
PIN_ENABLE_CLOCK( LED_3V3 );\
PIN_ENABLE_CLOCK( LED_1V8 );\
PIN_ENABLE_CLOCK( SW1 );\
PIN_ENABLE_CLOCK( VS_5V );\
PIN_ENABLE_CLOCK( VS_3V3 );\
PIN_ENABLE_CLOCK( VS_1V8 );\
\
PIN_INIT( OUTPUT_EN_5V  );\
PIN_INIT( OUTPUT_EN_3V3 );\
PIN_INIT( OUTPUT_EN_1V8 );\
PIN_INIT( LED_5V );\
PIN_INIT( LED_3V3 );\
PIN_INIT( LED_1V8 );\
PIN_INIT( SW1 );\
PIN_INIT( VS_5V );\
PIN_INIT( VS_3V3 );\
PIN_INIT( VS_1V8 );\
\
if( PIN_STAT( VS_5V ) == 0 ){ PIN_HI( LED_5V); PIN_HI( OUTPUT_EN_5V ); }\
if( PIN_STAT( VS_3V3 ) == 0 ){ PIN_HI( LED_3V3); PIN_HI( OUTPUT_EN_3V3 ); }\
if( PIN_STAT( VS_1V8 ) == 0 ){ PIN_HI( LED_1V8); PIN_HI( OUTPUT_EN_1V8 ); }\
}while(0)
#elif (defined USB2CAN)
/*
 * USB2CAN board variant (STM32F042C6T, nWorld mod)
 *
 * Pin assignment:
 *   CAN RX = PB8 (AF4)
 *   CAN TX = PB9 (AF4)
 *   LED TX = PB2  (active high)
 *   LED RX = PB10 (active high)
 *   LED STAT(LINK) = PB11 (active high)
 *
 * Clock: 16MHz external crystal
 */
#define IOPIN_TX    B, 2, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define IOPIN_RX    B, 10, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define IOPIN_STAT  B, 11, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define LED_ON      PIN_HI
#define LED_OFF     PIN_LOW

#define CAN_RX      B, 8, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN
#define CAN_TX      B, 9, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF4_CAN

#define pcan_variant_io_init() do{ \
  PIN_ENABLE_CLOCK( IOPIN_STAT ); \
  PIN_INIT( IOPIN_STAT ); \
}while(0)
#elif (defined G431_BOARD)
/*
 * MKS CANable V2.0 board variant (STM32G431C8)
 *
 * Pin assignment:
 *   FDCAN1_RX  = PB8  (AF9)
 *   FDCAN1_TX  = PB9  (AF9)
 *   USB D-     = PA11 (AF10)
 *   USB D+     = PA12 (AF10)
 *   LED D2 (blue/stat)  = PA15 (active low)
 *   LED D3 (green/work) = PA0  (active low)
 *
 * Clock: HSI 16MHz (no external crystal)
 */
#define IOPIN_TX    A, 15, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
#define IOPIN_RX    A, 0, MODE_OUTPUT_PP, NOPULL, SPEED_FREQ_MEDIUM, NOAF
/* Active low: ON = pin LOW, OFF = pin HIGH */
#define LED_ON      PIN_LOW
#define LED_OFF     PIN_HI

/* FDCAN1 pins: PB8 = RX (AF9), PB9 = TX (AF9) */
#define CAN_RX      B, 8, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF9_FDCAN1
#define CAN_TX      B, 9, MODE_AF_PP, NOPULL, SPEED_FREQ_HIGH, AF9_FDCAN1

#define pcan_variant_io_init()
#else
#error Unknown board variant
#endif

