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

bool SDCard::ReadThvFile(const char* fileName, float* data)
{
	FIL file;
	uint32_t readed;
	ThvFileHeader fileHeader;

	FRESULT res = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK)
	{
		res = f_read(&file, (void*)&fileHeader, sizeof(fileHeader), (UINT*)&readed);
		if ((res == FR_OK) && (readed == sizeof(fileHeader))) {
			res = f_read(&file, (void*)data, (fileHeader.width * fileHeader.height * 2), (UINT*)&readed);
		}
		if ((res == FR_OK) && (readed > 0)) {
			res = f_close(&file);
		}
	}

	return res == FR_OK;
}

bool SDCard::OpenDir(DIR* dir, const char* path)
{
	return f_opendir(dir, path) == FR_OK;
}

bool SDCard::CloseDir(DIR* dir)
{
	return f_closedir(dir) == FR_OK;
}

bool SDCard::ReadDir(DIR* dir, FILINFO* file)
{
	return f_readdir(dir, file) == FR_OK;
}

uint32_t SDCard::GetFilesCountInDir(const char* path)
{
	uint32_t result = 0;
	DIR dir;
	if (OpenDir(&dir, path))
	{
		FILINFO file;
		while (true)
		{
			if (!ReadDir(&dir, &file))
			{
				break;
			}
			if (file.fname[0] == 0)
			{
				break;
			}
			if (!(file.fattrib & AM_DIR)) //if not directory entry
			{
				result++;
			}
		}
		CloseDir(&dir);
	}
	return result;
}

bool SDCard::GetFileNameByIndex(const char* path, uint32_t index, const char* fileName)
{
	DIR dir;
	bool result = false;
	if (OpenDir(&dir, path))
	{
		FILINFO file;
		uint32_t i = 0;
		while (true)
		{
			if (!ReadDir(&dir, &file))
			{
				break;
			}
			if (file.fname[0] == 0)
			{
				break;
			}
			if (!(file.fattrib & AM_DIR)) //if not directory entry
			{
				if (i == index)
				{
					memcpy((void*)fileName, file.fname, sizeof(file.fname));
					result = true;
					break;
				}
				i++;
			}
		}
		CloseDir(&dir);
	}
	return result;
}

bool SDCard::isCardOk()
{
	return this->isInitOk;
}

