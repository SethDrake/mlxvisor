#pragma once 
#ifndef __UI_H
#define __UI_H

#include "ili9341.h"
#include "options.h"
#include "thermal.h"
#include <../CMSIS_RTOS/cmsis_os.h>
#include "portmacro.h"

#define DRAW_DELAY 15

#define THERMAL_SCALE 7

enum class UIScreen
{
	MAIN,
	SETTINGS,
	CONFIRM,
	FILES_LIST,
	FILE_VIEW
};

enum class Button
{
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3,
	OK = 4
};

class UI {
public:
	UI();
	~UI();
	void InitScreen(ILI9341* display, IRSensor* irSensor, Options* options);
	void DrawScreen();
	void ProcessButtons();
	void setScreen(UIScreen screen);
	void setButtonState(Button btn, bool isPressed);
	bool isButtonPressed(Button btn);
	bool isAnyButtonPressed();
protected:
	void DrawMainScreen();
	void DrawSettingsScreen();
	void DrawConfirmScreen();
	void DrawFilesListScreen();
	void DrawFileViewScreen();
	void DrawBattery();
	void DrawClock();

	const char* sensorRateToString(mlx90640_refreshrate_t rate);
private:
	UIScreen currentSreen;
	ILI9341* display;
	IRSensor* irSensor;
	Options* options;
	volatile uint8_t prevButtonsState = 0;
	volatile uint8_t buttonsState = 0;
	volatile TickType_t xDrawTime = 0;
	bool isStaticPartsRendered;
	uint8_t delayCntr = 0;

	uint16_t framebuffer[24 * THERMAL_SCALE * 32 * THERMAL_SCALE];
	uint16_t gradientFb[10 * 24 * THERMAL_SCALE];
};

#endif /* __UI_H */