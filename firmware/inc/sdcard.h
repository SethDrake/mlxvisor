#pragma once 
#ifndef __SDCARD_H
#define __SDCARD_H

#include "stm32f4xx_hal.h"
#include "ff.h"

class SDCard {
public:
	SDCard();
	~SDCard();
	void Init();
	bool isCardOk();
	bool SaveThvFile(const char* fileName, uint16_t width, uint16_t height, float* data);
protected:
	FATFS sdFatFs;
private:
	bool isInitOk = false;
	char SDPath[4];
};

#endif /* __SDCARD_H */ 