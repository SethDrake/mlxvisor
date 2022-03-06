﻿#include "ui.h"
#include <string.h>
#include "cpu_utils.h"

UI::UI()
{
	memset(framebuffer, 0x10, sizeof(framebuffer));
	memset(gradientFb, 0x10, sizeof(gradientFb));
	memset(batteryFb, 0x00, sizeof(batteryFb));

	this->currentSreen = UIScreen::MAIN;
	this->isStaticPartsRendered = false;
	this->display = NULL;
	this->irSensor = NULL;
	this->options = NULL;
	this->delayCntr = DRAW_DELAY;
	this->adcVbat = 0;
	this->_isSensorReadActive = true;
	this->selectedMenuItemIndex = 0;
	this->isMenuItemInEdit = false;
	this->activeSubMenuItemIndex = -1;
	this->preventDraw = false;

	menuItems[0] = {(uint8_t)MenuItems::DATE, "Date", 3};
	menuItems[1] = {(uint8_t)MenuItems::TIME, "Time", 3};
	menuItems[2] = {(uint8_t)MenuItems::EMISSION, "Emission", 1};
	menuItems[3] = {(uint8_t)MenuItems::SENSOR_RATE, "Sensor rate", 1};
	menuItems[4] = {(uint8_t)MenuItems::SENSOR_ADC_RESOLUTION, "Sensor ADC resolution", 1};
	menuItems[5] = {(uint8_t)MenuItems::COLOR_SCHEME, "Color scheme", 1};
	menuItems[6] = {(uint8_t)MenuItems::SHOW_MIN_TEMP_MARK, "Show min temp marker", 1};
	menuItems[7] = {(uint8_t)MenuItems::SHOW_MAX_TEMP_MARK, "Show max temp marker", 1};
	menuItems[8] = {(uint8_t)MenuItems::SHOW_CENTER_TEMP_MARK, "Show center temp marker", 1};
	menuItems[9] = {(uint8_t)MenuItems::BACK, "Back", 0};
}

UI::~UI()
{
}

void UI::InitScreen(ILI9341* display, IRSensor* irSensor, Options* options)
{
	this->display = display;
	this->irSensor = irSensor;
	this->options = options;
}

void UI::setScreen(UIScreen screen)
{
	preventDraw = true;
	selectedMenuItemIndex = 0;
	isMenuItemInEdit = false;
	delayCntr = DRAW_DELAY;

	this->currentSreen = screen;
	if (currentSreen == UIScreen::MAIN)
	{
		_isSensorReadActive = true;
	}
	else
	{
		_isSensorReadActive = false;
	}
	osDelay(100);
	this->isStaticPartsRendered = false;
	preventDraw = false;
}

void UI::setButtonState(Button btn, bool isPressed)
{
	prevButtonsState = buttonsState;
	if (isPressed)
	{
		buttonsState |= (1 << (int)btn);
	}
	else
	{
		buttonsState &= ~(1 << (int)btn);
	}
}

bool UI::isButtonPressed(Button btn)
{
	return (buttonsState & (1 << (int)btn)) != 0 && (prevButtonsState & (1 << (int)btn)) != 0;
}

bool UI::isAnyButtonPressed()
{
	return (buttonsState != 0) && (prevButtonsState != 0);
}

void UI::ProcessButtons()
{
	StoredOptionsDef_t* opts = options->GetCurrent();

	if (currentSreen == UIScreen::MAIN)
	{
		if (isButtonPressed(Button::UP))
		{
			if (opts->sensorRefreshRate == MLX90640_64_HZ)
			{
				opts->sensorRefreshRate = MLX90640_0_5_HZ;
			}
			else
			{
				opts->sensorRefreshRate = (mlx90640_refreshrate_t)((int)opts->sensorRefreshRate + 1);
			}
			options->SaveOptions();
			irSensor->setRefreshRate(opts->sensorRefreshRate);
		}
		else if (isButtonPressed(Button::LEFT))
		{
			if (opts->emission > 0.1)
			{
				opts->emission -= 0.05f;
			}
			options->SaveOptions();
		}
		else if (isButtonPressed(Button::RIGHT))
		{
			if (opts->emission < 1.0)
			{
				opts->emission += 0.05f;
			}
			options->SaveOptions();
		}
		else if (isButtonPressed(Button::DOWN))
		{
			setScreen(UIScreen::SETTINGS);
		}
		else if (isButtonPressed(Button::OK))
		{
			
		}
	}
	else if (currentSreen == UIScreen::SETTINGS)
	{
		if (isButtonPressed(Button::UP))
		{
			if (isMenuItemInEdit)
			{
				EditMenuItem((MenuItems)menuItems[selectedMenuItemIndex].id, Button::UP);
			}
			else {
				if (selectedMenuItemIndex > 0)
				{
					selectedMenuItemIndex--;
				}
			}
		}
		else if (isButtonPressed(Button::LEFT))
		{
			if (isMenuItemInEdit)
			{
				if (activeSubMenuItemIndex > 0)
				{
					activeSubMenuItemIndex--;
				}
			}
		}
		else if (isButtonPressed(Button::RIGHT))
		{
			if (isMenuItemInEdit)
			{
				if (activeSubMenuItemIndex < menuItems[selectedMenuItemIndex].subItemsCount-1)
				{
					activeSubMenuItemIndex++;
				}
			}
		}
		else if (isButtonPressed(Button::DOWN))
		{
			if (isMenuItemInEdit)
			{
				EditMenuItem((MenuItems)menuItems[selectedMenuItemIndex].id, Button::DOWN);
			}
			else {
				if (selectedMenuItemIndex < (MENU_ITEMS_COUNT - 1))
				{
					selectedMenuItemIndex++;
				}
			}
		}
		else if (isButtonPressed(Button::OK))
		{
			if (selectedMenuItemIndex == (int)MenuItems::BACK)
			{
				setScreen(UIScreen::MAIN);
			}
			else
			{
				isMenuItemInEdit = !isMenuItemInEdit;
				activeSubMenuItemIndex = isMenuItemInEdit ? 0 : -1;
			}
		}
	}

	if (isAnyButtonPressed())
	{
		delayCntr = DRAW_DELAY;
		buttonsState = 0;
	}
}

void UI::DrawBattery()
{
	if (adcVbat > 0) {
		memset(batteryFb, 0x00, sizeof(batteryFb));
		const float vBat = 0.3f + 2 * (3.3f * adcVbat) / 4096;
		uint16_t color = WHITE;
		if (vBat <= 3.6)
		{
			color = RED;
		}
		//draw bar: 3.2v=0;4.2v=42
		uint8_t progress = (uint8_t)((vBat - 3.2) * 42);
		progress = progress <= 43 ? progress : 42;
		progress = progress > 0 ? progress : 0;
		for(uint8_t i = 0; i < 10; i++)
		{
			display->drawHLineInBuf(batteryFb, 14, 3, i + 3, progress, DARK_GRAY);
		}
		display->printf(batteryFb, 14, 0, 0, color, "[%1.2f]", vBat);
		display->bufferDraw(274, 225, 48, 14, batteryFb);
	}
}

void UI::DrawClock()
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	GetDateTime(&date, &time);

	display->printf(240, 14, WHITE, BLACK, "%02u-%02u-20%02u", date.Date, date.Month, date.Year);
	display->printf(248, 0, WHITE, BLACK, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
}

void UI::DrawScreen()
{
	if (preventDraw)
	{
		return;
	}
	const TickType_t xTime1 = xTaskGetTickCount();
	if (delayCntr == DRAW_DELAY) {
		DrawBattery();
		DrawClock();
	}
	switch(currentSreen)
	{
		case UIScreen::MAIN: 
			DrawMainScreen();
			break;
		case UIScreen::SETTINGS: 
			DrawSettingsScreen();
			break;
		case UIScreen::DIALOG: 
			DrawConfirmScreen();
			break;
		case UIScreen::FILES_LIST: 
			DrawFilesListScreen();
			break;
		case UIScreen::FILE_VIEW: 
			DrawFileViewScreen();
			break;
	}
	xDrawTime = xTaskGetTickCount() - xTime1;
}

void UI::DrawMainScreen()
{
	const uint16_t centerX = 32 / 2 * THERMAL_SCALE;
	const uint16_t centerY = 24 / 2 * THERMAL_SCALE;
	const StoredOptionsDef_t* opts = options->GetCurrent();

	//runtime content
	irSensor->VisualizeImage(framebuffer, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, 1); //prepare thermal image in framebuffer
	//draw central mark with temp in framebuffer
	if (opts->showCenterTempMarker) {
		display->drawMarkInBuf(framebuffer, 24 * THERMAL_SCALE, centerX, centerY, WHITE);
		display->printf(framebuffer, 24 * THERMAL_SCALE, centerX - 18, centerY - 25, WHITE, "%.1f\x81", irSensor->getCenterTemp());
	}

	//draw framebuffers and other content
	//static content
	if (!isStaticPartsRendered)
	{
		irSensor->DrawGradient(gradientFb, 10, 24 * THERMAL_SCALE);
		display->bufferDraw(32 * THERMAL_SCALE, 239 - 24 * THERMAL_SCALE, 10, 24 * THERMAL_SCALE, gradientFb);

		//buttons description
		display->printf(290, 140, YELLOW, BLACK, "R"); //up
		display->printf(272, 125, YELLOW, BLACK, "E-"); //left
		display->printf(290, 125, YELLOW, BLACK, "S"); //center
		display->printf(303, 125, YELLOW, BLACK, "E+"); //right
		display->printf(290, 110, YELLOW, BLACK, "M"); //down

		isStaticPartsRendered = true;
	}

	//info content
	if (delayCntr >= DRAW_DELAY) {
		const int16_t maxTemp = (int16_t)irSensor->getMaxTemp();
		const int16_t minTemp = (int16_t)irSensor->getMinTemp();

		//right panel
		display->fillScreen(235, 225, 235 + 40, 225 + 14, BLACK); 
		display->printf(235, 225, WHITE, BLACK, "%d\x81", maxTemp);
		display->fillScreen(235, 68, 235 + 40, 68 + 14, BLACK);
		display->printf(235, 68, GREEN, BLACK, "%d\x81", minTemp);

		//status string
		const uint16_t cpuUsage = osGetCPUUsage();
		display->printf(0, 14, WHITE, BLACK, "E=%.2f Rate=%s", opts->emission, sensorRateToString(opts->sensorRefreshRate));
		display->printf(0, 0, WHITE, BLACK, "Frm:%03ums CPU:%02u%%", xDrawTime, cpuUsage);

		delayCntr = 0;
	}
	delayCntr++;

	//thermal image famebuffer
	display->bufferDraw(0, 239 - 24 * THERMAL_SCALE, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, framebuffer);
}

void UI::DrawSettingsScreen()
{
	if (!isStaticPartsRendered)
	{
		display->clear(BLACK);
		display->printf(10, 225, WHITE, BLACK, "SETTINGS");

		isStaticPartsRendered = true;
	}

	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	GetDateTime(&date, &time);

	const StoredOptionsDef_t* opts = options->GetCurrent();

	for (uint8_t i = 0; i < MENU_ITEMS_COUNT; i++)
	{
		const uint16_t lineStartY = 210 - 14 * (i + 1);
		const uint16_t fCol = GetMenuFrontColor(menuItems[i].id, selectedMenuItemIndex, true);
		const uint16_t bCol = GetMenuBackColor(menuItems[i].id, selectedMenuItemIndex, true);
		
		display->printf(10, lineStartY, fCol, bCol, "%s", menuItems[i].name);

		//draw submenu
		const uint16_t shift = strlen(menuItems[i].name) * 8 + 20;
		const bool isActualMenuItemInEdit = (menuItems[i].id == selectedMenuItemIndex) && isMenuItemInEdit;
		if (menuItems[i].id == (int)MenuItems::DATE)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%02u", date.Date);
			display->printf(shift + 2*8, lineStartY, WHITE, BLACK, "-");
			DrawSubItem(shift + 3 * 8, lineStartY, 1, isActualMenuItemInEdit, "%02u", date.Month);
			display->printf(shift + 5*8, lineStartY, WHITE, BLACK, "-");
			DrawSubItem(shift + 6 * 8, lineStartY, 2, isActualMenuItemInEdit, "20%02u", date.Year);
		}
		else if (menuItems[i].id == (int)MenuItems::TIME)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%02u", time.Hours);
			display->printf(shift + 2*8, lineStartY, WHITE, BLACK, ":");
			DrawSubItem(shift + 3 * 8, lineStartY, 1, isActualMenuItemInEdit, "%02u", time.Minutes);
			display->printf(shift + 5*8, lineStartY, WHITE, BLACK, ":");
			DrawSubItem(shift + 6 * 8, lineStartY, 2, isActualMenuItemInEdit, "%02u", time.Seconds);	
		}
		else if (menuItems[i].id == (int)MenuItems::EMISSION)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%1.2f", opts->emission);
		}
		else if (menuItems[i].id == (int)MenuItems::SENSOR_RATE)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", sensorRateToString(opts->sensorRefreshRate));
		}
		else if (menuItems[i].id == (int)MenuItems::SENSOR_ADC_RESOLUTION)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", sensorAdcResolutionToString(opts->sensorAdcResolution));
		}
		else if (menuItems[i].id == (int)MenuItems::COLOR_SCHEME)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", colorSchemeToString(opts->colorScheme));
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_MIN_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", opts->showMinTempMarker ? "Y" : "N");
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_MAX_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", opts->showMaxTempMarker ? "Y" : "N");
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_CENTER_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%s", opts->showCenterTempMarker ? "Y" : "N");
		}
	}
}

void UI::DrawConfirmScreen()
{
}

void UI::DrawFilesListScreen()
{
}

void UI::DrawFileViewScreen()
{
}

bool UI::isSensorReadActive()
{
	return _isSensorReadActive;
}

void UI::DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, uint8_t val)
{
	display->printf(x, y, GetMenuFrontColor(0, activeSubMenuItemIndex, inEdit), GetMenuBackColor(0, activeSubMenuItemIndex, inEdit), format, val);
}

void UI::DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, float val)
{
	display->printf(x, y, GetMenuFrontColor(0, activeSubMenuItemIndex, inEdit), GetMenuBackColor(0, activeSubMenuItemIndex, inEdit), format, val);
}

void UI::DrawSubItem(uint16_t x, uint16_t y, uint8_t subMenuIndex, bool inEdit, const char* format, const char* val)
{
	display->printf(x, y, GetMenuFrontColor(0, activeSubMenuItemIndex, inEdit), GetMenuBackColor(0, activeSubMenuItemIndex, inEdit), format, val);
}

void UI::EditMenuItem(MenuItems menuItem, Button button)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	GetDateTime(&date, &time);
	StoredOptionsDef_t* opts = options->GetCurrent();

	if (menuItem == MenuItems::DATE)
	{
		if (activeSubMenuItemIndex == 0)
		{
			if (button == Button::UP)
			{
				date.Date++;
				if (date.Date > GetDaysInMonth(date.Month, date.Year + 2000))
				{
					date.Date = GetDaysInMonth(date.Month, date.Year + 2000);
				}
			}
			else if (button == Button::DOWN)
			{
				date.Date--;
				if (date.Date < 1)
				{
					date.Date = 1;
				}
			}
		}
		else if (activeSubMenuItemIndex == 1)
		{
			if (button == Button::UP)
			{
				date.Month++;
				if (date.Month > 12)
				{
					date.Month = 12;
				}
			}
			else if (button == Button::DOWN)
			{
				date.Month--;
				if (date.Month < 1)
				{
					date.Month = 1;
				}
			}
		}
		else if (activeSubMenuItemIndex == 2)
		{
			if (button == Button::UP)
			{
				date.Year++;
				if (date.Year > 99)
				{
					date.Year = 99;
				}
			}
			else if (button == Button::DOWN)
			{
				date.Year--;
				if (date.Year < 1)
				{
					date.Year = 1;
				}
			}
		}

		SaveDateTime(&date, &time);
	}
	else if (menuItem == MenuItems::TIME)
	{
		if (activeSubMenuItemIndex == 0)
		{
			if (button == Button::UP)
			{
				time.Hours++;
				if (time.Hours > 23)
				{
					time.Hours = 23;
				}
			}
			else if (button == Button::DOWN)
			{
				time.Hours--;
			}
		}
		else if (activeSubMenuItemIndex == 1)
		{
			if (button == Button::UP)
			{
				time.Minutes++;
				if (time.Minutes > 59)
				{
					time.Minutes = 59;
				}
			}
			else if (button == Button::DOWN)
			{
				time.Minutes--;
			}
		}
		else if (activeSubMenuItemIndex == 2)
		{
			if (button == Button::UP)
			{
				time.Seconds++;
				if (time.Seconds > 59)
				{
					time.Seconds = 59;
				}
			}
			else if (button == Button::DOWN)
			{
				time.Seconds--;
			}
		}
		SaveDateTime(&date, &time);
	}
	else if (menuItem == MenuItems::EMISSION)
	{
		if (button == Button::UP)
		{
			if (opts->emission < 1.0)
			{
				opts->emission += 0.05f;
			}
		}
		else if (button == Button::DOWN)
		{
			if (opts->emission > 0.1)
			{
				opts->emission -= 0.05f;
			}
		}
	}
	else if (menuItem == MenuItems::SENSOR_RATE)
	{
		if (button == Button::UP)
		{
			if (opts->sensorRefreshRate < MLX90640_64_HZ)
			{
				opts->sensorRefreshRate = (mlx90640_refreshrate_t)((int)opts->sensorRefreshRate + 1);
			}
		}
		else if (button == Button::DOWN)
		{
			if (opts->sensorRefreshRate > MLX90640_0_5_HZ)
			{
				opts->sensorRefreshRate = (mlx90640_refreshrate_t)((int)opts->sensorRefreshRate - 1);
			}
		}
		irSensor->setRefreshRate(opts->sensorRefreshRate);
	}
	else if (menuItem == MenuItems::SENSOR_ADC_RESOLUTION)
	{
		if (button == Button::UP)
		{
			if (opts->sensorAdcResolution < MLX90640_ADC_19BIT)
			{
				opts->sensorAdcResolution = (mlx90640_resolution_t)((int)opts->sensorAdcResolution + 1);
			}
		}
		else if (button == Button::DOWN)
		{
			if (opts->sensorAdcResolution > MLX90640_ADC_16BIT)
			{
				opts->sensorAdcResolution = (mlx90640_resolution_t)((int)opts->sensorAdcResolution - 1);
			}
		}
		irSensor->setRefreshRate(opts->sensorRefreshRate);
	}
	else if (menuItem == MenuItems::COLOR_SCHEME)
	{
		if (opts->colorScheme == DEFAULT_SCHEME)
		{
			opts->colorScheme = ALTERNATE_SCHEME;
		}
		else
		{
			opts->colorScheme = DEFAULT_SCHEME;
		}
		irSensor->setColorScheme(opts->colorScheme);
	}
	else if (menuItem == MenuItems::SHOW_MIN_TEMP_MARK)
	{
		opts->showMinTempMarker = !opts->showMinTempMarker;
	}
	else if (menuItem == MenuItems::SHOW_MAX_TEMP_MARK)
	{
		opts->showMaxTempMarker = !opts->showMaxTempMarker;
	}
	else if (menuItem == MenuItems::SHOW_CENTER_TEMP_MARK)
	{
		opts->showCenterTempMarker = !opts->showCenterTempMarker;
	}

	options->SaveOptions();
}

uint16_t UI::GetMenuFrontColor(int8_t menuIndex, int8_t activeMenuIndex, bool isActive)
{
	return isActive && (menuIndex == activeMenuIndex) ? BLACK : WHITE;
}

uint16_t UI::GetMenuBackColor(int8_t menuIndex, int8_t activeMenuIndex, bool isActive)
{
	return isActive && (menuIndex == activeMenuIndex) ? WHITE : BLACK;
}

uint8_t UI::GetDaysInMonth(uint8_t month, uint16_t year)
{
	uint8_t	daysInMonth = days_in_month[month - 1];
	if (month == 2)
	{
		if ((year % 4 == 0) && ((year % 4 != 0) || (year % 400 == 0))) //is leap year
		{
			daysInMonth++;
		}
	}
	return daysInMonth;
}

const char* UI::sensorRateToString(mlx90640_refreshrate_t rate)
{
	switch(rate)
	{
		case MLX90640_0_5_HZ: return "0.5Hz";
		case MLX90640_1_HZ: return   "1Hz  ";
		case MLX90640_2_HZ: return   "2Hz  ";
		case MLX90640_4_HZ: return   "4Hz  ";
		case MLX90640_8_HZ: return   "8Hz  ";
		case MLX90640_16_HZ: return  "16Hz ";
		case MLX90640_32_HZ: return  "32Hz ";
		case MLX90640_64_HZ: return  "64Hz ";
	}
	return "";
}

const char* UI::sensorAdcResolutionToString(mlx90640_resolution_t resolution)
{
	switch (resolution)
	{
		case MLX90640_ADC_16BIT: return "16bit";
		case MLX90640_ADC_17BIT: return "17bit";
		case MLX90640_ADC_18BIT: return "18bit";
		case MLX90640_ADC_19BIT: return "19bit";
	}
	return "";
}

const char* UI::colorSchemeToString(thermal_colorscheme_t scheme)
{
	switch (scheme)
	{
	case DEFAULT_SCHEME: return   "Default  ";
	case ALTERNATE_SCHEME: return "Alternate";
	}
	return "";
}