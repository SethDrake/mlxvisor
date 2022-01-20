#pragma once 
#ifndef __HARDWARE_H
#define __HARDWARE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define I2C_TIMEOUT_MAX   0x3000 /*<! The value of the maximal timeout for I2C waiting loops */

#define I2C1_SPEED        800000

#define I2C1_SCL_PORT	  GPIOB
#define I2C1_SDA_PORT	  GPIOB
#define I2C1_SCL_PIN	  GPIO_PIN_6
#define I2C1_SDA_PIN	  GPIO_PIN_7

#define USR_BTN_PORT	  GPIOA
#define USR_BTN_PIN		  GPIO_PIN_0

#define USR_LED_PORT	  GPIOD
#define USR_LED1_PIN	  GPIO_PIN_12
#define USR_LED2_PIN	  GPIO_PIN_13
#define USR_LED3_PIN	  GPIO_PIN_14
#define USR_LED4_PIN	  GPIO_PIN_15
  
extern I2C_HandleTypeDef  i2c1;

void	Clock_Init(void);  
void	GPIO_Init(void);
void	I2C_Init(void);

void      GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, uint8_t state);
void      GPIO_TogglePin(GPIO_TypeDef* port, uint16_t pin);
uint8_t   GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);

void      I2Cx_WriteData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t Value);
void      I2Cx_WriteData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t Value);
void      I2Cx_WriteBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
void      I2Cx_WriteBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t Length);
uint8_t   I2Cx_ReadData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg);
uint16_t  I2Cx_ReadData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg);
uint8_t   I2Cx_ReadBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
uint8_t   I2Cx_ReadBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_H */