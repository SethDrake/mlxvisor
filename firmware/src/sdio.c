#include "sdio.h"
#include "hardware.h"

#if defined(SDMMC_DATATIMEOUT)
#define SD_TIMEOUT SDMMC_DATATIMEOUT
#elif defined(SD_DATATIMEOUT)
#define SD_TIMEOUT SD_DATATIMEOUT
#else
#define SD_TIMEOUT 30 * 1000
#endif

#define SD_DEFAULT_BLOCK_SIZE 512


static volatile DSTATUS Stat = STA_NOINIT;

DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize(BYTE);
DSTATUS SD_status(BYTE);
DRESULT SD_read(BYTE, BYTE*, DWORD, UINT);
DRESULT SD_write(BYTE, const BYTE*, DWORD, UINT);
DRESULT SD_ioctl(BYTE, BYTE, void*);

const Diskio_drvTypeDef  SD_Driver =
{
	SD_initialize,
	SD_status,
	SD_read,
	SD_write,
	SD_ioctl,
};

DSTATUS SD_CheckStatus(BYTE lun)
{
	Stat = STA_NOINIT;

	if (SD_GetCardState() == MSD_OK)
	{
		Stat &= ~STA_NOINIT;
	}

	return Stat;
}

DSTATUS SD_initialize(BYTE lun)
{
	Stat = STA_NOINIT;
	if (SDCard_Init() == MSD_OK)
	{
		Stat = SD_CheckStatus(lun);
	}
	return Stat;
}

DSTATUS SD_status(BYTE lun)
{
	return SD_CheckStatus(lun);
}

DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;

	if (SD_ReadBlocks((uint32_t*)buff,
		(uint32_t)(sector),
		count,
		SD_TIMEOUT) == MSD_OK)
	{
		/* wait until the read operation is finished */
		while (SD_GetCardState() != MSD_OK)
		{
		}
		res = RES_OK;
	}

	return res;
}

DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;

	if (SD_WriteBlocks((uint32_t*)buff,
		(uint32_t)(sector),
		count,
		SD_TIMEOUT) == MSD_OK)
	{
		/* wait until the Write operation is finished */
		while (SD_GetCardState() != MSD_OK)
		{
		}
		res = RES_OK;
	}

	return res;
}

DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;
	HAL_SD_CardInfoTypeDef cardInfo;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd)
	{
		/* Make sure that no pending write process */
	case CTRL_SYNC :
		res = RES_OK;
		break;

		/* Get number of sectors on the disk (DWORD) */
	case GET_SECTOR_COUNT :
		SD_GetCardInfo(&cardInfo);
		*(DWORD*)buff = cardInfo.LogBlockNbr;
		res = RES_OK;
		break;

		/* Get R/W sector size (WORD) */
	case GET_SECTOR_SIZE :
		SD_GetCardInfo(&cardInfo);
		*(WORD*)buff = cardInfo.LogBlockSize;
		res = RES_OK;
		break;

		/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
		SD_GetCardInfo(&cardInfo);
		*(DWORD*)buff = cardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}

uint8_t SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	if (HAL_SD_ReadBlocks(&sdio, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK)
	{
		return MSD_ERROR;
	}
	return MSD_OK;
}


uint8_t SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	if (HAL_SD_WriteBlocks(&sdio, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK)
	{
		return MSD_ERROR;
	}
	return MSD_OK;
}

uint8_t SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{  
	/* Read block(s) in DMA transfer mode */
	if (HAL_SD_ReadBlocks_DMA(&sdio, (uint8_t *)pData, ReadAddr, NumOfBlocks) != HAL_OK)
	{
		return MSD_ERROR;
	}
	return MSD_OK;
}

uint8_t SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{ 
	/* Write block(s) in DMA transfer mode */
	if (HAL_SD_WriteBlocks_DMA(&sdio, (uint8_t *)pData, WriteAddr, NumOfBlocks) != HAL_OK)
	{
		return MSD_ERROR;
	}
	return MSD_OK;
}

uint8_t SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
	if (HAL_SD_Erase(&sdio, StartAddr, EndAddr) != HAL_OK)
	{
		return MSD_ERROR;
	}
	return MSD_OK;
}

uint8_t SD_GetCardState()
{
	return ((HAL_SD_GetCardState(&sdio) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}
  

void SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardInfo)
{
	HAL_SD_GetCardInfo(&sdio, cardInfo);
}
