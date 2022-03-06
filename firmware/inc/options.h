#pragma once 
#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <cstdint>
#include "mlx90640.h"
#include "stm32f4xx_hal.h"

#define STORED_OPTS_MAGIC 0xF00D

typedef struct StoredOptionsDef_t
{
	uint16_t magic;
	float emission;
	mlx90640_refreshrate sensorRefreshRate;
	mlx90640_resolution sensorAdcResolution;
	thermal_colorscheme_t colorScheme;
	bool showMinTempMarker;
	bool showMaxTempMarker;
	bool showCenterTempMarker;
} StoredOptions_t;

class Options
{
public:
	Options();
	~Options();
	void LoadOptions();
	void SaveOptions();
	StoredOptions_t* GetCurrent();
protected:
	void InitDefaultOptions();
private:
	StoredOptions_t storedOptions;
};

#endif /* __OPTIONS_H */