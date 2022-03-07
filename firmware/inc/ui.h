#pragma once 
#ifndef __UI_H
#define __UI_H

#include "ili9341.h"
#include "options.h"
#include "thermal.h"
#include "sdcard.h"
#include <../CMSIS_RTOS/cmsis_os.h>
#include "portmacro.h"

#define DRAW_DELAY 15

#define THERMAL_SCALE 7

enum class UIScreen
{
	MAIN,
	SETTINGS,
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

enum class MenuItems
{
	DATE = 0,
	TIME = 1,
	EMISSION = 2,
	SENSOR_RATE = 3,
	SENSOR_ADC_RESOLUTION = 4,
	COLOR_SCHEME = 5,
	SHOW_MIN_TEMP_MARK = 6,
	SHOW_MAX_TEMP_MARK = 7,
	SHOW_CENTER_TEMP_MARK = 8,
	BACK = 9
};

#define MENU_ITEMS_COUNT 10

#define MAX_FILES_ON_SCREEN 14

static const uint8_t days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

typedef struct MenuItemDef_t
{
	uint8_t id;
	const char* name;
	uint8_t subItemsCount;
} menuItem_t;

class UI {
public:
	UI();
	~UI();
	void InitScreen(ILI9341* display, IRSensor* irSensor, SDCard* sdCard, Options* options);
	void DrawScreen();
	void ProcessButtons();
	void setScreen(UIScreen screen);
	void setButtonState(Button btn, bool isPressed);
	bool isButtonPressed(Button btn);
	bool isAnyButtonPressed();
	bool isSensorReadActive();
	__IO uint16_t adcVbat;
protected:
	void DrawMainScreen();
	void DrawSettingsScreen();
	void DrawFilesListScreen();
	void DrawFileViewScreen();
	void DrawBattery();
	void DrawClock();
private:
	const char* sensorRateToString(mlx90640_refreshrate_t rate);
	const char* sensorAdcResolutionToString(mlx90640_resolution_t resolution);
	const char* colorSchemeToString(thermal_colorscheme_t scheme);
	uint16_t GetMenuFrontColor(int8_t menuIndex, int8_t activeMenuIndex, bool isActive);
	uint16_t GetMenuBackColor(int8_t menuIndex, int8_t activeMenuIndex, bool isActive);
	void EditMenuItem(MenuItems menuItem, Button button);
	uint8_t GetDaysInMonth(uint8_t month, uint16_t year);
	void DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, uint8_t val);
	void DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, float val);
	void DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, const char* val);
	volatile UIScreen currentSreen;
	ILI9341* display;
	IRSensor* irSensor;
	Options* options;
	SDCard* sdCard;
	uint16_t backgroundColor = BLACK;
	volatile uint8_t prevButtonsState = 0;
	volatile uint8_t buttonsState = 0;
	volatile TickType_t xDrawTime = 0;
	volatile bool isStaticPartsRendered;
	volatile bool _isSensorReadActive;
	volatile bool preventDraw;
	volatile uint8_t delayCntr;
	char statusLine[100] = {0};

	char selectedFileName[_MAX_LFN + 1] = {0};
	volatile uint32_t filesCount = 0;

	volatile int32_t selectedItemIndex;

	volatile bool isMenuItemInEdit;
	volatile int8_t activeSubMenuItemIndex;
	menuItem_t menuItems[MENU_ITEMS_COUNT];
};

#endif /* __UI_H */