#include "options.h"
#include <string.h>


Options::Options()
{
	InitDefaultOptions();
}

Options::~Options()
{
}

void Options::InitDefaultOptions()
{
	storedOptions.magic = STORED_OPTS_MAGIC;
	storedOptions.emission = 0.95f;
	storedOptions.sensorRefreshRate = MLX90640_16_HZ;
	storedOptions.sensorAdcResolution = MLX90640_ADC_18BIT;
	storedOptions.colorScheme = ALTERNATE_SCHEME;
	storedOptions.showCenterTempMarker = true;
	storedOptions.showMaxTempMarker = true;
	storedOptions.showMinTempMarker = true;
}

StoredOptions_t* Options::GetCurrent()
{
	return &storedOptions;
}

void Options::LoadOptions()
{
	memcpy((void*)&storedOptions, (void*)BKPSRAM_BASE, sizeof(StoredOptions_t));
	if (storedOptions.magic != STORED_OPTS_MAGIC)
	{
		InitDefaultOptions();
	}
}

void Options::SaveOptions()
{
	memcpy((void*)BKPSRAM_BASE, (void*)&storedOptions, sizeof(StoredOptions_t));
}
