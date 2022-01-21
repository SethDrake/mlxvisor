#pragma once 
#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define I2C_TIMEOUT_MAX   0x3000

void				I2Cx_WriteData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t Value);
void				I2Cx_WriteData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t Value);
void				I2Cx_WriteBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t size);
void				I2Cx_WriteBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t size);
uint8_t				I2Cx_ReadData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg);
uint16_t			I2Cx_ReadData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg);
HAL_StatusTypeDef   I2Cx_ReadBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t size);
HAL_StatusTypeDef   I2Cx_ReadBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t size);
uint8_t				I2Cx_ScanBus(I2C_HandleTypeDef* i2c, uint8_t* foundAddresses);

extern void	  I2Cx_Error(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H */