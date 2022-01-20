#pragma once
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/ 
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define THERMAL_SCALE 7

extern void Error_Handler(uint8_t source);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


