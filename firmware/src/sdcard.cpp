#include "sdcard.h"
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
			isInitOk = true;
		}
	}
}

bool SDCard::SaveThvFile(const char* fileName, uint16_t width, uint16_t height, float* data)
{
	FIL file;
	uint32_t written;

	FRESULT res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
	if (res == FR_OK)
	{
		res = f_write(&file, (const void*)&width, sizeof(width), (UINT*)&written);
		if (res == FR_OK) {
			res =  f_write(&file, (const void*)&height, sizeof(height), (UINT*)&written);
		}
		if (res == FR_OK) {
			res = f_write(&file, (const void*)data, width * height, (UINT*)&written);
		}
		f_close(&file);
	}

	return res == FR_OK;
}

bool SDCard::isCardOk()
{
	return this->isInitOk;
}

