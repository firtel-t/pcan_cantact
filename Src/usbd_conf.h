#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(STM32G431xx)
#include "stm32g4xx.h"
#include "stm32g4xx_hal.h"
#else
#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"
#endif

#define USBD_SUPPORT_USER_STRING_DESC (1)
#define USBD_DEBUG_LEVEL      0
#define DEVICE_FS             0

extern PCD_HandleTypeDef hpcd_USB_FS;
