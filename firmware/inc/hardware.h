#pragma once 
#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define I2C1_SPEED          1000000
#define SPI1_PRESCALER      SPI_BAUDRATEPRESCALER_2
#define SPI1_PRESCALER_SLOW SPI_BAUDRATEPRESCALER_16

#define I2C1_SCL_PORT		GPIOB
#define I2C1_SCL_PIN		GPIO_PIN_6
#define I2C1_SDA_PORT		GPIOB
#define I2C1_SDA_PIN		GPIO_PIN_7

#define SPI1_MOSI_PORT		GPIOA
#define SPI1_MOSI_PIN		GPIO_PIN_7
#define SPI1_MISO_PORT		GPIOA
#define SPI1_MISO_PIN		GPIO_PIN_6
#define SPI1_SCK_PORT		GPIOA
#define SPI1_SCK_PIN		GPIO_PIN_5

#define SPI1_TX_DMA_CHL     DMA_CHANNEL_3
#define SPI1_TX_DMA_STRM    DMA2_Stream3
#define SPI1_TX_DMA_IRQ     DMA2_Stream3_IRQn

#define ADC1_DMA_CHL        DMA_CHANNEL_2
#define ADC1_DMA_STRM       DMA2_Stream0
#define ADC1_DMA_IRQ        DMA2_Stream0_IRQn

#define SD_TX_RX_DMA_CHL    DMA_CHANNEL_4
#define SD_TX_RX_DMA_STRM   DMA2_Stream6  
#define SD_TX_RX_IRQn       DMA2_Stream6_IRQn

#define LCD_CS_PORT			GPIOC
#define LCD_CS_PIN			GPIO_PIN_4
#define LCD_DC_PORT			GPIOC
#define LCD_DC_PIN			GPIO_PIN_5

#define USR_BTN_OK_PORT		GPIOB
#define USR_BTN_OK_PIN		GPIO_PIN_4
#define USR_BTN_U_PORT		GPIOB
#define USR_BTN_U_PIN		GPIO_PIN_3
#define USR_BTN_R_PORT		GPIOD
#define USR_BTN_R_PIN		GPIO_PIN_7
#define USR_BTN_D_PORT		GPIOD
#define USR_BTN_D_PIN		GPIO_PIN_6
#define USR_BTN_L_PORT		GPIOD
#define USR_BTN_L_PIN		GPIO_PIN_5

#define SD_CLKDIV		    8 
#define SD_CLKDIV_SLOW	    5

#define SDIO_D0_PORT		GPIOC
#define SDIO_D0_PIN			GPIO_PIN_8
#define SDIO_D1_PORT		GPIOC
#define SDIO_D1_PIN			GPIO_PIN_9
#define SDIO_D2_PORT		GPIOC
#define SDIO_D2_PIN			GPIO_PIN_10
#define SDIO_D3_PORT		GPIOC
#define SDIO_D3_PIN			GPIO_PIN_11
#define SDIO_CK_PORT		GPIOC
#define SDIO_CK_PIN			GPIO_PIN_12
#define SDIO_CMD_PORT		GPIOD
#define SDIO_CMD_PIN		GPIO_PIN_2

#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)

#define RTC_ASYNCH_PREDIV       127
#define RTC_SYNCH_PREDIV        255
  
extern I2C_HandleTypeDef	i2c1;
extern SPI_HandleTypeDef	spi1;
extern RTC_HandleTypeDef	rtc;
extern ADC_HandleTypeDef	adc1;
extern SD_HandleTypeDef     sdio;

void	Clock_Init(void);
void	RTC_Config(void);
void	GPIO_Init(void);
void	I2C_Init(void);
void	SPI_Init(void);
void	ADC_Init(void);
uint8_t SDCard_Init(void);
void	DMA_Init(void);

void    GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, uint8_t state);
void    GPIO_TogglePin(GPIO_TypeDef* port, uint16_t pin);
uint8_t GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);

void	GetDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time);
void	SaveDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time);

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_H */