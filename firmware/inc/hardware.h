#pragma once 
#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define I2C1_SPEED          800000
#define SPI1_PRESCALER      SPI_BAUDRATEPRESCALER_2
#define SPI1_PRESCALER_SLOW SPI_BAUDRATEPRESCALER_16

#define I2C1_SCL_PORT		GPIOA
#define I2C1_SCL_PIN		GPIO_PIN_8
#define I2C1_SDA_PORT		GPIOC
#define I2C1_SDA_PIN		GPIO_PIN_9

#define SPI1_MOSI_PORT		GPIOF
#define SPI1_MOSI_PIN		GPIO_PIN_9
#define SPI1_MISO_PORT		GPIOF
#define SPI1_MISO_PIN		GPIO_PIN_8
#define SPI1_SCK_PORT		GPIOF
#define SPI1_SCK_PIN		GPIO_PIN_7

#define LCD_CS_PORT			GPIOC
#define LCD_CS_PIN			GPIO_PIN_2
#define LCD_DC_PORT			GPIOD
#define LCD_DC_PIN			GPIO_PIN_13
#define LCD_RESET_PORT		GPIOA
#define LCD_RESET_PIN		GPIO_PIN_3

#define USR_BTN_PORT		GPIOA
#define USR_BTN_PIN			GPIO_PIN_0

#define USR_LED1_PORT		GPIOG
#define USR_LED1_PIN		GPIO_PIN_13
#define USR_LED2_PORT		GPIOG
#define USR_LED2_PIN		GPIO_PIN_14
  
extern I2C_HandleTypeDef	i2c1;
extern SPI_HandleTypeDef	spi1;

void	Clock_Init(void);  
void	GPIO_Init(void);
void	I2C_Init(void);
void	SPI_Init(void);

void      GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, uint8_t state);
void      GPIO_TogglePin(GPIO_TypeDef* port, uint16_t pin);
uint8_t   GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_H */