#pragma once 
#ifndef __THERMAL_H
#define __THERMAL_H

#include <stm32f4xx_hal.h>
#include <mlx90640.h>

#define THERM_COEFF 0.0625
#define TEMP_COEFF 0.25
#define SUPPLY_VOLTAGE 3.3f
#define RECALC_DELAY 15

class IRSensor {
public:
	IRSensor();
	~IRSensor();
	bool Init(I2C_HandleTypeDef* i2c, mlx90640_refreshrate_t rate, mlx90640_resolution_t resolution, const uint8_t* colorScheme);
	void Reset();
	void ReadImage();
	void VisualizeImage(uint16_t* fb, uint16_t sizeX, uint16_t sizeY, uint8_t method);
	void DrawGradient(uint16_t* fb, uint16_t sizeX, uint16_t sizeY);
	void CalculateTempMap(float emissivity);
	void FindMinAndMaxTemp();
	void setColorScheme(const uint8_t* colorScheme);
	uint16_t* readSerialNumber();
	void readMlxEE();
	void convertMlxEEToParams();
	mlx90640_refreshrate_t getRefreshRate();
	void setRefreshRate(mlx90640_refreshrate_t rate);
	mlx90640_resolution_t getADCResolution();
	void setADCResolution(mlx90640_resolution_t resolution);
	mlx90640_mode_t readMlxMode();
	void setMlxMode(mlx90640_mode_t mode);
	bool isFrameReady();
	bool isImageReady();
	uint16_t getSubPage();
	float getVdd();
	float getTa();
	float* getTempMap();
	float getMaxTemp();
	float getMinTemp();
	float getCenterTemp();
	uint16_t getHotDotIndex();
	uint16_t getColdDotIndex();
	uint16_t temperatureToRGB565(float temperature, float minTemp, float maxTemp);
protected:
	mlx90640_refreshrate_t readRefreshRate();
	mlx90640_resolution_t readADCResolution();
	uint16_t rgb2color(uint8_t R, uint8_t G, uint8_t B);
	uint8_t calculateRGB(uint8_t rgb1, uint8_t rgb2, float t1, float step, float t);
	uint8_t IsPixelBad(uint16_t index);
	uint8_t CheckAdjacentPixels(uint16_t pix1, uint16_t pix2);
private:
	I2C_HandleTypeDef* i2c;
	volatile bool _isImageReady;
	uint16_t fbSizeX;
	uint16_t fbSizeY;
	const uint8_t* colorScheme;
	paramsMLX90640_t mlxParams;
	mlx90640_refreshrate_t rate;
	mlx90640_resolution_t resolution;
	uint16_t mlxEE[832];
	uint16_t frameData[834];
	uint16_t recalcCnt;
	float vdd;
	float ta;
	float dots[24*32];
	uint16_t colors[24*32];
	uint16_t coldDotIndex;
	uint16_t hotDotIndex;
	float minTemp;
	float maxTemp;
	float centerTemp;
	const float minTempCorr = -0.5;
	const float maxTempCorr = 0.5;

	uint16_t mlxSerialNumber[3];
};


static const uint8_t DEFAULT_COLOR_SCHEME[] = {
	 28, 1, 108 ,
	 31, 17, 218 ,
	 50, 111, 238 ,
	 63, 196, 229 ,
	 64, 222, 135 ,
	 192, 240, 14 ,
	 223, 172, 18 ,
	 209, 111, 14 ,
	 210, 50, 28 ,
	 194, 26, 0 ,
	 132, 26, 0 
};

static const uint8_t ALTERNATE_COLOR_SCHEME[] = {
	 0, 0, 5 ,
	 7, 1, 97 ,
	 51, 1, 194 ,
	 110, 2, 212 ,
	 158, 6, 150 ,
	 197, 30, 58 ,
	 218, 66, 0 ,
	 237, 137, 0 ,
	 246, 199, 23 ,
	 251, 248, 117 ,
	 252, 254, 253 
};

#endif /* __THERMAL_H */


