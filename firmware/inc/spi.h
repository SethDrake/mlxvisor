#pragma once 
#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_TIMEOUT_MAX		((uint32_t)0x1000)

void			    SPIx_WriteData(SPI_HandleTypeDef* spi, uint8_t value);
void			    SPIx_WriteData16(SPI_HandleTypeDef* spi, uint16_t value);
void			    SPIx_WriteBuffer(SPI_HandleTypeDef* spi, uint8_t *pBuffer, uint16_t size);
uint8_t			    SPIx_ReadData(SPI_HandleTypeDef* spi);
uint16_t		    SPIx_ReadData16(SPI_HandleTypeDef* spi);
HAL_StatusTypeDef   SPIx_ReadBuffer(SPI_HandleTypeDef* spi, uint8_t *pBuffer, uint16_t size);
void				SPIx_SetPrescaler(SPI_HandleTypeDef* spi, uint32_t prescaler);
void				SPIx_SetDataSize(SPI_HandleTypeDef* spi, uint32_t dataSize);

extern void	  SPIx_Error(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H */