#pragma once 
#ifndef __SDIO_H
#define __SDIO_H

#include "stm32f4xx_hal.h"
#include "ff_gen_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)    
#define   SD_PRESENT                    ((uint8_t)0x01)
#define   SD_NOT_PRESENT                ((uint8_t)0x00)
		  						        
#define   SD_DATATIMEOUT                ((uint32_t)100000000)

extern const Diskio_drvTypeDef  SD_Driver;

uint8_t SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks);
uint8_t SD_Erase(uint32_t StartAddr, uint32_t EndAddr);
uint8_t SD_GetCardState();
void    SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardInfo);

#ifdef __cplusplus
}
#endif

#endif /* __SDIO_H */