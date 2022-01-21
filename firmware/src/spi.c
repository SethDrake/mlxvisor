#include <spi.h>

void SPIx_WriteData(SPI_HandleTypeDef* spi, uint8_t value)
{
	SPIx_WriteBuffer(spi, &value, 1);
}

void SPIx_WriteData16(SPI_HandleTypeDef* spi, uint16_t value)
{
	SPIx_WriteBuffer(spi, (uint8_t*)&value, 2);
}

void SPIx_WriteBuffer(SPI_HandleTypeDef* spi, uint8_t* pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_SPI_Transmit(spi, pBuffer, size, SPI_TIMEOUT_MAX);
	if (status != HAL_OK)
	{
		SPIx_Error();
	}
}

uint8_t SPIx_ReadData(SPI_HandleTypeDef* spi)
{
	uint8_t value = 0;
	SPIx_ReadBuffer(spi, &value, 1);
  
	return value;
}

uint16_t SPIx_ReadData16(SPI_HandleTypeDef* spi)
{
	uint16_t value = 0;
	SPIx_ReadBuffer(spi, (uint8_t*)&value, 2);
  
	return value;
}

HAL_StatusTypeDef SPIx_ReadBuffer(SPI_HandleTypeDef* spi, uint8_t* pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_SPI_Receive(spi, pBuffer, size, SPI_TIMEOUT_MAX);
	if (status != HAL_OK)
	{
		SPIx_Error();
	}
  
	return status;
}

void SPIx_SetPrescaler(SPI_HandleTypeDef* spi, uint32_t prescaler)
{
	spi->Init.BaudRatePrescaler = prescaler;
	HAL_SPI_Init(spi);
}

void SPIx_SetDataSize(SPI_HandleTypeDef* spi, uint32_t dataSize)
{
	spi->Init.DataSize = dataSize;
	HAL_SPI_Init(spi);
}
