#pragma once 
#ifndef __SDCARD_H
#define __SDCARD_H

#include "stm32f4xx_hal.h"
#include "ff.h"

#define THV_FILE_MAGIC 0x9E6DE8DA

typedef struct ThvFileHeader_t
{
	uint32_t magic;
	uint16_t width;
	uint16_t height;
} ThvFileHeader;

class SDCard {
public:
	SDCard();
	~SDCard();
	void Init();
	bool isCardOk();
	bool SaveThvFile(const char* fileName, uint16_t width, uint16_t height, float* data);
	uint32_t GetFilesCountInDir(const char* path);
	bool OpenDir(DIR* dir, const char* path);
	bool CloseDir(DIR* dir);
	bool ReadDir(DIR* dir, FILINFO* file);
protected:
	FATFS sdFatFs;
	bool calculateFreeSpace();
private:
	volatile bool isInitOk = false;
	char SDPath[4];
	volatile uint32_t totalSectors;
	volatile uint32_t freeSectors;
};

#endif /* __SDCARD_H */ 