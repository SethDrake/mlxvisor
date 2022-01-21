#include <i2c.h>

void I2Cx_WriteData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT_MAX); 
  
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteData16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t Value)
{
	uint16_t val = __builtin_bswap16(Value);
  
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&val, 2, I2C_TIMEOUT_MAX); 
  
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, size, I2C_TIMEOUT_MAX); 

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

void I2Cx_WriteBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t size)
{
	// swap bytes
	for (uint16_t i = 0; i < size / 2; i++)
	{
		pBuffer[i] = __builtin_bswap16(pBuffer[i]);
	}	
    
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pBuffer, size, I2C_TIMEOUT_MAX); 

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
	}        
}

uint8_t I2Cx_ReadData(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg)
{
	uint8_t value = 0;
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_MAX);
 
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
	uint16_t value = 0;
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&value, 2, I2C_TIMEOUT_MAX);
 
	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();
  
	}
	return __builtin_bswap16(value);
}

HAL_StatusTypeDef I2Cx_ReadBuffer(I2C_HandleTypeDef* i2c, uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, size, I2C_TIMEOUT_MAX);
  
	/* Check the communication status */
	if (status == HAL_OK)
	{
		return status;
	}
	else
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();

		return status;
	}
}

HAL_StatusTypeDef I2Cx_ReadBuffer16(I2C_HandleTypeDef* i2c, uint8_t Addr, uint16_t Reg, uint16_t *pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, Addr, Reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pBuffer, size, I2C_TIMEOUT_MAX);
  
	/* Check the communication status */
	if (status == HAL_OK)
	{
		// swap bytes
		for (uint16_t i = 0; i < size / 2; i++)
		{
			pBuffer[i] = __builtin_bswap16(pBuffer[i]);
		}	

		return status;
	}
	else
	{
		/* Re-Initialize the BUS */
		I2Cx_Error();

		return status;
	}
}

uint8_t I2Cx_ScanBus(I2C_HandleTypeDef* i2c, uint8_t* foundAddresses)
{
	uint8_t size = 0;

	for (uint8_t i = 1; i < 128; i++)
	{
		const uint8_t devAddress = (i << 1);
		const HAL_StatusTypeDef ret = HAL_I2C_IsDeviceReady(i2c, devAddress, 3, 5);
		if (ret == HAL_OK)
		{
			foundAddresses[size] = i;
			size++;
		}
	}

	return size;
}
