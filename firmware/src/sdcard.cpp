﻿#include "sdcard.h"
#include "sdio.h"
#include "ff_gen_drv.h"
#include <string.h>


SDCard::SDCard()
{
	memset(&SDPath, 0x00, sizeof(SDPath));
	this->isInitOk = false;
}

SDCard::~SDCard()
{
	FATFS_UnLinkDriver(SDPath);
}

void SDCard::Init()
{
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	{
		if (f_mount(&sdFatFs, (TCHAR const*)SDPath, 0) == FR_OK)
		{
			isInitOk = calculateFreeSpace();
		}
	}
}

bool SDCard::calculateFreeSpace()
{
	uint32_t freClusters;
	FATFS* ff = &sdFatFs;

	FRESULT res = f_getfree((const TCHAR*)"0:", (DWORD*)&freClusters, &ff);
	totalSectors = (sdFatFs.n_fatent - 2) * sdFatFs.csize;
	freeSectors = freClusters * sdFatFs.csize;
	return res == FR_OK;
}

bool SDCard::SaveThvFile(const char* fileName, uint16_t width, uint16_t height, float* data)
{
	FIL file;
	uint32_t written;

	const ThvFileHeader fileHeader = {THV_FILE_MAGIC, width, height};

	FRESULT res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
	if (res == FR_OK)
	{
		res = f_write(&file, (const void*)&fileHeader, sizeof(fileHeader), (UINT*)&written);
		if ((res == FR_OK) && (written > 0)) {
			res = f_write(&file, (const void*)data, (width * height * 2), (UINT*)&written);
		}
		if ((res == FR_OK) && (written > 0)) {
			res = f_close(&file);
		}
	}

	return res == FR_OK;
}

bool SDCard::isCardOk()
{
	return this->isInitOk;
}

