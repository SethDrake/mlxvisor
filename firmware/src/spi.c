#include <spi.h>

void SPIx_WriteData(SPI_HandleTypeDef* spi, uint8_t value)
{
	SPIx_WriteBuffer(spi, &value, 1);
}

HAL_StatusTypeDef SPIx_WriteData16(SPI_HandleTypeDef* spi, uint16_t value)
{
	//SPIx_WriteBuffer(spi, (uint8_t*)&value, spi->Init.DataSize == SPI_DATASIZE_16BIT ? 1 : 2);

	__HAL_LOCK(spi);
	spi->State       = HAL_SPI_STATE_BUSY_TX;
	spi->Instance->DR = value;
	while (__HAL_SPI_GET_FLAG(spi, SPI_FLAG_BSY) != RESET){};
	__HAL_UNLOCK(spi);

	return HAL_OK;
}

void SPIx_WriteBuffer(SPI_HandleTypeDef* spi, uint8_t* pBuffer, uint16_t size)
{
	const HAL_StatusTypeDef status = HAL_SPI_Transmit(spi, pBuffer, size, SPI_TIMEOUT_MAX);
	if (status != HAL_OK)
	{
		SPIx_Error();
	}
}

void SPIx_WriteBufferDMA(SPI_HandleTypeDef* spi, uint16_t* pBuffer, uint32_t size)
{
	/*SPIx_SetDataSize(spi, SPI_DATASIZE_16BIT);
	SPIx_Enable(spi);
	for (uint32_t l = 0; l < size; l++) {
		SPIx_WriteData16(spi, pBuffer[l]);
	}
	SPIx_SetDataSize(spi, SPI_DATASIZE_8BIT);*/

	SPIx_SetDataSize(spi, SPI_DATASIZE_16BIT);
	const HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(spi, (uint8_t*)pBuffer, size);
	if (status != HAL_OK)
	{
		SPIx_SetDataSize(spi, SPI_DATASIZE_8BIT);
		SPIx_Error();
	}
	else 
	{
		while (spi->State != HAL_SPI_STATE_READY){}; //wait for the end of sending data to SPI
		SPIx_SetDataSize(spi, SPI_DATASIZE_8BIT);
	}

	const uint16_t max_size = 65535;
	if (size > max_size)
	{
		const uint16_t remain = size - max_size;
		SPIx_WriteBufferDMA(spi, (pBuffer + max_size), remain);
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
	SPIx_ReadBuffer(spi, (uint8_t*)&value, spi->Init.DataSize == SPI_DATASIZE_16BIT ? 1 : 2);
  
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

void SPIx_Enable(SPI_HandleTypeDef* spi)
{
	if ((spi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
	{
		/* Enable SPI peripheral */
		__HAL_SPI_ENABLE(spi);
	}
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
