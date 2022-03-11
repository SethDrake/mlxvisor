#include "usbd_storage.h"
#include "sdio.h"

#define STORAGE_LUN_NBR                  1

const int8_t STORAGE_Inquirydata[] = {/* 36 */
  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'S', 'E', 'T', 'H', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'M', 'L', 'X', 'V', 'i', 's', 'o', 'r', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '2' ,'7'                      /* Version      : 4 Bytes */
};

int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t * block_num, uint16_t * block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t * buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t * buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

int8_t STORAGE_Init(uint8_t lun)
{
	return SD_Driver.disk_initialize(lun);
}

int8_t STORAGE_GetCapacity(uint8_t lun,
	uint32_t * block_num,
	uint16_t * block_size)
{
	HAL_SD_CardInfoTypeDef cardInfo;
	int8_t ret = -1;

	if (SD_Driver.disk_status(lun) != STA_NOINIT)
	{
		SD_GetCardInfo(&cardInfo);

		*block_num = cardInfo.LogBlockNbr - 1;
		*block_size = cardInfo.LogBlockSize;
		ret = 0;
	}
	return ret;
}

int8_t STORAGE_IsReady(uint8_t lun)
{
	static int8_t prev_status = 0;
	int8_t ret = -1;

	if (SD_Driver.disk_status(lun) != STA_NOINIT)
	{
		if (prev_status < 0)
		{
			SD_Driver.disk_initialize(lun);
			prev_status = 0;

		}
		if (SD_GetCardState() == SD_TRANSFER_OK)
		{
			ret = 0;
		}
	}
	else if (prev_status == 0)
	{
		prev_status = -1;
	}
	return ret;
}

int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
	return 0;
}

int8_t STORAGE_Read(uint8_t lun, uint8_t * buf, uint32_t blk_addr, uint16_t blk_len)
{
	return SD_Driver.disk_read(lun, buf, blk_addr, blk_len);
}

int8_t STORAGE_Write(uint8_t lun, uint8_t * buf, uint32_t blk_addr, uint16_t blk_len)
{
	return SD_Driver.disk_write(lun, buf, blk_addr, blk_len);
}

int8_t STORAGE_GetMaxLun(void)
{
	return (STORAGE_LUN_NBR - 1);
}
