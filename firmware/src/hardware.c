#include <hardware.h>

I2C_HandleTypeDef i2c1;

static void	I2Cx_Error(void);

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
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
 
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
	clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

void GPIO_Init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	/* Enable GPIO clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/* Configure the Common GPIOs */
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pin = USR_BTN_PIN; //BTN
	HAL_GPIO_Init(USR_BTN_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pin = USR_LED1_PIN; //LED1
	HAL_GPIO_Init(USR_LED_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED2_PIN; //LED2
	HAL_GPIO_Init(USR_LED_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED3_PIN; //LED3
	HAL_GPIO_Init(USR_LED_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = USR_LED4_PIN; //LED4
	HAL_GPIO_Init(USR_LED_PORT, &GPIO_InitStruct);

	/* Configure I2C Pins */
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	GPIO_InitStruct.Pin       = I2C1_SCL_PIN; //SCL
	HAL_GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStruct); 
	GPIO_InitStruct.Pin		  = I2C1_SDA_PIN; //SDA
	HAL_GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStruct);
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

void GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, uint8_t state)
{
	HAL_GPIO_WritePin(port, pin, (GPIO_PinState)state); 
}

void GPIO_TogglePin(GPIO_TypeDef* port, uint16_t pin)
{
	HAL_GPIO_TogglePin(port, pin);
}

uint8_t GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin)
{
	return HAL_GPIO_ReadPin(port, pin);
}

void I2Cx_WriteData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	HAL_StatusTypeDef status = HAL_OK;
  
	status = HAL_I2C_Mem_Write(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT_MAX); 
  
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t Value)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint16_t val = __builtin_bswap16(Value);
  
	status = HAL_I2C_Mem_Write(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&val, 2, I2C_TIMEOUT_MAX); 
  
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
	HAL_StatusTypeDef status = HAL_OK;
  
	status = HAL_I2C_Mem_Write(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2C_TIMEOUT_MAX); 

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t Length)
{
	HAL_StatusTypeDef status = HAL_OK;

	// swap bytes
	for (uint16_t i = 0; i < Length / 2; i++)
	{
		pBuffer[i] = __builtin_bswap16(pBuffer[i]);
	}	
    
	status = HAL_I2C_Mem_Write(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pBuffer, Length, I2C_TIMEOUT_MAX); 

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

uint8_t I2Cx_ReadData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t value = 0;
  
	status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_MAX);
 
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
  
	}
	return value;
}

uint16_t I2Cx_ReadData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint16_t value = 0;
  
	status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&value, 2, I2C_TIMEOUT_MAX);
 
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
  
	}
	return __builtin_bswap16(value);
}

uint8_t I2Cx_ReadBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Read(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2C_TIMEOUT_MAX);
  
	/* Check the communication status */
	if (status == HAL_OK)
	{
		return 0;
	}
	else
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();

		return 1;
	}
}

uint8_t I2Cx_ReadBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t Length)
{
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pBuffer, Length, I2C_TIMEOUT_MAX);
  
	/* Check the communication status */
	if (status == HAL_OK)
	{
		// swap bytes
		for (uint16_t i = 0; i < Length / 2; i++)
		{
			pBuffer[i] = __builtin_bswap16(pBuffer[i]);
		}	

		return 0;
	}
	else
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();

		return 1;
	}
}

static void I2Cx_Error()
{
	/* De-initialize the I2C communication BUS */
	HAL_I2C_DeInit(&i2c1);
  
	/* Re-Initialize the I2C communication BUS */
	I2C_Init();
}