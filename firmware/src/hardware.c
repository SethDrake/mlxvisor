#include <hardware.h>

I2C_HandleTypeDef i2c1;
SPI_HandleTypeDef spi1;
RTC_HandleTypeDef rtc;
ADC_HandleTypeDef adc1;

void Clock_Init()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
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

	/* Enable Backup SRAM */
	__HAL_RCC_BKPSRAM_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();
	// Enable backup regulator
	HAL_PWREx_EnableBkUpReg(); 

	/* Enable RTC */
	__HAL_RCC_RTC_ENABLE();

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK) {
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) == HAL_OK)
		{
			rtc.Instance = RTC;
			rtc.Init.HourFormat = RTC_HOURFORMAT_24;
			rtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
			rtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
			rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
			rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
			rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
			if (HAL_RTC_Init(&rtc) == HAL_OK) {
				RTC_Config();
			}
		}
	}
}

void RTC_Config()
{
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime = { 0 };

	//HAL_RTCEx_BKUPWrite(&rtc, RTC_BKP_DR1, 0xF10D);

	//init calendar if not already configured
	if (HAL_RTCEx_BKUPRead(&rtc, RTC_BKP_DR1) != 0xF00D)
	{
		sTime.Hours = 15;
		sTime.Minutes = 45;
		sTime.Seconds = 0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;

		sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
		sDate.Month = RTC_MONTH_MARCH;
		sDate.Date = 2;
		sDate.Year = 22;

		SaveDateTime(&sDate, &sTime);

		HAL_RTCEx_BKUPWrite(&rtc, RTC_BKP_DR1, 0xF00D);
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
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pin = USR_BTN_OK_PIN; 
	HAL_GPIO_Init(USR_BTN_OK_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_BTN_U_PIN; 
	HAL_GPIO_Init(USR_BTN_U_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_BTN_R_PIN; 
	HAL_GPIO_Init(USR_BTN_R_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_BTN_D_PIN; 
	HAL_GPIO_Init(USR_BTN_D_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_BTN_L_PIN; 
	HAL_GPIO_Init(USR_BTN_L_PORT, &GPIO_InitStruct);

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
		spi1.Instance = SPI1;
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
		__HAL_RCC_SPI1_CLK_ENABLE();
	} 
}

void ADC_Init()
{
	ADC_ChannelConfTypeDef sConfig;

	adc1.Instance          = ADC1;
	adc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
	adc1.Init.Resolution = ADC_RESOLUTION_12B;
	adc1.Init.ScanConvMode = DISABLE;
	adc1.Init.ContinuousConvMode = DISABLE;
	adc1.Init.DiscontinuousConvMode = DISABLE;
	adc1.Init.NbrOfDiscConversion = 0;
	adc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	adc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	adc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adc1.Init.NbrOfConversion = 1;
	adc1.Init.DMAContinuousRequests = DISABLE;
	adc1.Init.EOCSelection = DISABLE;

	__HAL_RCC_ADC1_CLK_ENABLE();

	if (HAL_ADC_Init(&adc1) == HAL_OK)
	{
		sConfig.Channel = ADC_CHANNEL_VBAT;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
		sConfig.Offset = 0;
		HAL_ADC_ConfigChannel(&adc1, &sConfig);
	}
  
	
}

void DMA_Init()
{
	static DMA_HandleTypeDef hdmaSpi;
	static DMA_HandleTypeDef hdmaAdc;

	__HAL_RCC_DMA2_CLK_ENABLE();

	hdmaSpi.Instance                 = SPI1_TX_DMA_STRM;
	hdmaSpi.Init.Channel             = SPI1_TX_DMA_CHL;
	hdmaSpi.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdmaSpi.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdmaSpi.Init.MemInc              = DMA_MINC_ENABLE;
	hdmaSpi.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdmaSpi.Init.MemDataAlignment    = DMA_PDATAALIGN_HALFWORD;
	hdmaSpi.Init.Mode                = DMA_NORMAL;
	hdmaSpi.Init.Priority            = DMA_PRIORITY_LOW;
	hdmaSpi.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
	hdmaSpi.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdmaSpi.Init.MemBurst            = DMA_MBURST_INC4;
	hdmaSpi.Init.PeriphBurst         = DMA_PBURST_INC4;
  
	HAL_DMA_Init(&hdmaSpi);   
  
	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(&spi1, hdmatx, hdmaSpi);

	HAL_NVIC_SetPriority(SPI1_TX_DMA_IRQ, 0, 1);
	HAL_NVIC_EnableIRQ(SPI1_TX_DMA_IRQ);


	hdmaAdc.Instance                 = ADC1_DMA_STRM;
	hdmaAdc.Init.Channel             = ADC1_DMA_CHL;
	hdmaAdc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdmaAdc.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdmaAdc.Init.MemInc              = DMA_MINC_ENABLE;
	hdmaAdc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdmaAdc.Init.MemDataAlignment    = DMA_PDATAALIGN_WORD;
	hdmaAdc.Init.Mode                = DMA_NORMAL;
	hdmaAdc.Init.Priority            = DMA_PRIORITY_LOW;
	hdmaAdc.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
	hdmaAdc.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdmaAdc.Init.MemBurst            = DMA_MBURST_SINGLE;
	hdmaAdc.Init.PeriphBurst         = DMA_PBURST_SINGLE;
  
	/*HAL_DMA_Init(&hdmaAdc);   

	__HAL_LINKDMA(&adc1, DMA_Handle, hdmaAdc);

	HAL_NVIC_SetPriority(ADC1_DMA_IRQ, 1, 1);
	HAL_NVIC_EnableIRQ(ADC1_DMA_IRQ);*/
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

void GetDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time)
{
	HAL_RTC_GetTime(&rtc, time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc, date, RTC_FORMAT_BIN);
}

void SaveDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time)
{
	HAL_RTC_SetTime(&rtc, time, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&rtc, date, RTC_FORMAT_BIN);
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
