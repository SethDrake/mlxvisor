#include <hardware.h>

I2C_HandleTypeDef i2c1;
SPI_HandleTypeDef spi1;

void Clock_Init()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
  
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
  
	/* The voltage scaling allows optimizing the power consumption when the device is 
		clocked below the maximum system frequency, to update the voltage scaling value 
		regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 240;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 10;
	// RCC_OscInitStruct.PLL.PLLM = 8;
	// RCC_OscInitStruct.PLL.PLLN = 336;
	// RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	// RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Activate the Over-Drive mode */
	// HAL_PWREx_EnableOverDrive();
 
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
	clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	/* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
	if (HAL_GetREVID() == 0x1001)
	{
		/* Enable the Flash prefetch */
		__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	}
}

void GPIO_Init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	/* Enable GPIO clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	// __HAL_RCC_GPIOF_CLK_ENABLE();
	// __HAL_RCC_GPIOG_CLK_ENABLE();

	/* Configure I2C Pins */
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	GPIO_InitStruct.Pin       = I2C1_SCL_PIN; //SCL
	HAL_GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStruct); 
	GPIO_InitStruct.Pin		  = I2C1_SDA_PIN; //SDA
	HAL_GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStruct);

	/* Configure SPI Pins */
	GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull   = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	GPIO_InitStruct.Pin    = SPI1_MOSI_PIN; //MOSI
	HAL_GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin    = SPI1_MISO_PIN; //MISO
	HAL_GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin    = SPI1_SCK_PIN; //SCK
	HAL_GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStruct);

	/* Configure the Common GPIOs */
	/* Input */
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pin = USR_BTN_PIN; //BTN
	HAL_GPIO_Init(USR_BTN_PORT, &GPIO_InitStruct);

	/* Output */
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = LCD_CS_PIN; //LCD CS
	HAL_GPIO_Init(LCD_CS_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LCD_DC_PIN; //LCD DC
	HAL_GPIO_Init(LCD_DC_PORT, &GPIO_InitStruct);
	// GPIO_InitStruct.Pin = LCD_RESET_PIN; //LCD RESET
	// HAL_GPIO_Init(LCD_RESET_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pin = USR_LED1_PIN; //LED1
	HAL_GPIO_Init(USR_LED1_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED2_PIN; //LED2
	HAL_GPIO_Init(USR_LED2_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED3_PIN; //LED3
	HAL_GPIO_Init(USR_LED3_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED4_PIN; //LED4
	HAL_GPIO_Init(USR_LED4_PORT, &GPIO_InitStruct);
}

void I2C_Init()
{
	if (HAL_I2C_GetState(&i2c1) == HAL_I2C_STATE_RESET)
	{
		i2c1.Instance              = I2C1;
		i2c1.Init.ClockSpeed       = I2C1_SPEED;
		i2c1.Init.DutyCycle        = I2C_DUTYCYCLE_2;
		i2c1.Init.OwnAddress1      = 0;
		i2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
		i2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
		i2c1.Init.OwnAddress2      = 0;
		i2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
		i2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;  

		/* Enable I2C1 clock */
		__HAL_RCC_I2C1_CLK_ENABLE();
		__HAL_RCC_I2C1_FORCE_RESET();
		__HAL_RCC_I2C1_RELEASE_RESET();

		HAL_I2C_Init(&i2c1);
	}
}

void SPI_Init()
{
	if (HAL_SPI_GetState(&spi1) == HAL_SPI_STATE_RESET)
	{
		/* SPI configuration -----------------------------------------------------*/
		spi1.Instance = SPI2;
		spi1.Init.BaudRatePrescaler = SPI1_PRESCALER;
		spi1.Init.Direction      = SPI_DIRECTION_2LINES;
		spi1.Init.CLKPhase       = SPI_PHASE_1EDGE;
		spi1.Init.CLKPolarity    = SPI_POLARITY_LOW;
		spi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
		spi1.Init.CRCPolynomial  = 7;
		spi1.Init.DataSize       = SPI_DATASIZE_8BIT;
		spi1.Init.FirstBit       = SPI_FIRSTBIT_MSB;
		spi1.Init.NSS            = SPI_NSS_SOFT;
		spi1.Init.TIMode         = SPI_TIMODE_DISABLED;
		spi1.Init.Mode           = SPI_MODE_MASTER;

		/* Enable SPI1 clock */
		__HAL_RCC_SPI2_CLK_ENABLE();
	} 
}

void DMA_Init()
{
	static DMA_HandleTypeDef hdma;

	hdma.Instance                 = SPI1_TX_DMA_STRM;
	hdma.Init.Channel             = SPI1_TX_DMA_CHL;
	hdma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma.Init.MemInc              = DMA_MINC_ENABLE;
	hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma.Init.MemDataAlignment    = DMA_PDATAALIGN_HALFWORD;
	hdma.Init.Mode                = DMA_NORMAL;
	hdma.Init.Priority            = DMA_PRIORITY_LOW;
	hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
	hdma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma.Init.MemBurst            = DMA_MBURST_INC4;
	hdma.Init.PeriphBurst         = DMA_PBURST_INC4;

	__HAL_RCC_DMA1_CLK_ENABLE();
  
	HAL_DMA_Init(&hdma);   
  
	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(&spi1, hdmatx, hdma);

	HAL_NVIC_SetPriority(SPI1_TX_DMA_IRQ, 0, 1);
	HAL_NVIC_EnableIRQ(SPI1_TX_DMA_IRQ);

	// HAL_NVIC_SetPriority(SPI1_IRQn, 0, 2);
	// HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

void GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, uint8_t state)
{
	HAL_GPIO_WritePin(port, pin, state); 
}

void GPIO_TogglePin(GPIO_TypeDef* port, uint16_t pin)
{
	HAL_GPIO_TogglePin(port, pin);
}

uint8_t GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin)
{
	return HAL_GPIO_ReadPin(port, pin);
}

void I2Cx_Error(void)
{
	/* De-initialize the I2C communication BUS */
	HAL_I2C_DeInit(&i2c1);
  
	/* Re-Initialize the I2C communication BUS */
	I2C_Init();
}

void SPIx_Error(void)
{
	/* De-initialize the SPI communication BUS */
	HAL_SPI_DeInit(&spi1);
  
	/* Re-Initialize the SPI communication BUS */
	SPI_Init();
}
