// ReSharper disable CppClangTidyClangDiagnosticSignCompare
#include "ui.h"
#include "hardware.h"
#include <cstdio>
#include <string.h>
#include "cpu_utils.h"

UI::UI()
{
	this->currentSreen = UIScreen::MAIN;
	this->isStaticPartsRendered = false;
	this->display = NULL;
	this->irSensor = NULL;
	this->sdCard = NULL;
	this->options = NULL;
	this->delayCntr = DRAW_DELAY;
	this->adcVbat = 0;
	this->_isSensorReadActive = true;
	this->selectedItemIndex = 0;
	this->isMenuItemInEdit = false;
	this->activeSubMenuItemIndex = -1;
	this->preventDraw = false;
	this->isCharging = false;

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

void UI::InitScreen(ILI9341* display, IRSensor* irSensor, SDCard* sdCard, Options* options)
{
	this->display = display;
	this->irSensor = irSensor;
	this->sdCard = sdCard;
	this->options = options;
}

void UI::setScreen(UIScreen screen)
{
	preventDraw = true;
	isMenuItemInEdit = false;
	
	this->currentSreen = screen;
	if (currentSreen == UIScreen::MAIN)
	{
		_isSensorReadActive = true;
		backgroundColor = BLACK;
		selectedItemIndex = 0;
	}
	else
	{
		_isSensorReadActive = false;
		backgroundColor = DARK_BLUE;
	}
	if (currentSreen == UIScreen::FILES_LIST)
	{
		if (sdCard->isCardOk()) {
			filesCount = sdCard->GetFilesCountInDir("");
		}
	}
	osDelay(100);
	this->isStaticPartsRendered = false;
	preventDraw = false;
	delayCntr = DRAW_DELAY;
}

UIScreen UI::GetScreen()
{
	return currentSreen;
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
			setScreen(UIScreen::FILES_LIST);
		}
		else if (isButtonPressed(Button::RIGHT))
		{
			
		}
		else if (isButtonPressed(Button::DOWN))
		{
			setScreen(UIScreen::SETTINGS);
		}
		else if (isButtonPressed(Button::OK))
		{
			if (sdCard->isCardOk())
			{
				char fileName[30];
				RTC_TimeTypeDef time;
				RTC_DateTypeDef date;

				GetDateTime(&date, &time);
				sprintf(fileName, "20%02u%02u%02u-%02u%02u%02u.thv", date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);

				if (sdCard->SaveThvFile((const char*)&fileName, 32, 24, irSensor->getTempMap()))
				{
					sprintf(statusLine, "File %s saved.", fileName);
				}
				else
				{
					sprintf(statusLine, "Error on save file.            ");
				}
			}
		}
	}
	else if (currentSreen == UIScreen::SETTINGS)
	{
		if (isButtonPressed(Button::UP))
		{
			if (isMenuItemInEdit)
			{
				EditMenuItem((MenuItems)menuItems[selectedItemIndex].id, Button::UP);
			}
			else {
				if (selectedItemIndex > 0)
				{
					selectedItemIndex--;
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
				if (activeSubMenuItemIndex < menuItems[selectedItemIndex].subItemsCount-1)
				{
					activeSubMenuItemIndex++;
				}
			}
		}
		else if (isButtonPressed(Button::DOWN))
		{
			if (isMenuItemInEdit)
			{
				EditMenuItem((MenuItems)menuItems[selectedItemIndex].id, Button::DOWN);
			}
			else {
				if (selectedItemIndex < (MENU_ITEMS_COUNT - 1))
				{
					selectedItemIndex++;
				}
			}
		}
		else if (isButtonPressed(Button::OK))
		{
			if (selectedItemIndex == (int)MenuItems::BACK)
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
	else if (currentSreen == UIScreen::FILES_LIST)
	{
		if (isButtonPressed(Button::UP))
		{
			if (selectedItemIndex > 0)
			{
				selectedItemIndex--;
			}
			else
			{
				selectedItemIndex = filesCount - 1;
			}
		}
		else if (isButtonPressed(Button::DOWN))
		{
			if (selectedItemIndex < (filesCount - 1))
			{
				selectedItemIndex++;
			}
			else
			{
				selectedItemIndex = 0;
			}
		}
		else if (isButtonPressed(Button::LEFT) || isButtonPressed(Button::RIGHT))
		{
			setScreen(UIScreen::MAIN);
		}
		else if (isButtonPressed(Button::OK))
		{
			if (sdCard->GetFileNameByIndex("", selectedItemIndex, selectedFileName)) {
				setScreen(UIScreen::FILE_VIEW);
			}
		}
	}
	else if (currentSreen == UIScreen::FILE_VIEW)
	{
		if (isButtonPressed(Button::UP))
		{
			if (selectedItemIndex > 0)
			{
				selectedItemIndex--;
			}
			else
			{
				selectedItemIndex = filesCount - 1;
			}
			if (sdCard->GetFileNameByIndex("", selectedItemIndex, selectedFileName)) {
				setScreen(UIScreen::FILE_VIEW);
			}
		}
		if (isButtonPressed(Button::DOWN))
		{
			if (selectedItemIndex < (filesCount - 1))
			{
				selectedItemIndex++;
			}
			else
			{
				selectedItemIndex = 0;
			}
			if (sdCard->GetFileNameByIndex("", selectedItemIndex, selectedFileName)) {
				setScreen(UIScreen::FILE_VIEW);
			}
		}
		if (isButtonPressed(Button::LEFT) || isButtonPressed(Button::RIGHT) || isButtonPressed(Button::OK))
		{
			setScreen(UIScreen::FILES_LIST);
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
		for (uint16_t i = 0; i < sizeof(batteryFb)/2; i++)
		{
			batteryFb[i] = backgroundColor;
		}

		const float vBat = 0.3f + 2 * (3.146f * adcVbat) / 4096;
		uint16_t color = WHITE;
		if (vBat <= 3.6)
		{
			color = RED;
		}
		if (isCharging)
		{
			color = BLUE;
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

	display->printf(240, 14, WHITE, backgroundColor, "%02u-%02u-20%02u", date.Date, date.Month, date.Year);
	display->printf(248, 0, WHITE, backgroundColor, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
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
		case UIScreen::FILES_LIST: 
			DrawFilesListScreen();
			break;
		case UIScreen::FILE_VIEW: 
			DrawFileViewScreen();
			break;
		case UIScreen::USB_CONNECTED_MODE: 
			DrawUsbConnectedScreen();
			break;
	}
	xDrawTime = xTaskGetTickCount() - xTime1;
}

void UI::DrawMainScreen()
{
	const uint16_t centerX = 32 / 2 * THERMAL_SCALE;
	const uint16_t centerY = 24 / 2 * THERMAL_SCALE;
	const StoredOptionsDef_t* opts = options->GetCurrent();

	//draw framebuffers and other content
	//static content
	if (!isStaticPartsRendered)
	{
		display->clear(backgroundColor);
		irSensor->DrawGradient(gradientFb, 10, 24 * THERMAL_SCALE);
		display->bufferDraw(32 * THERMAL_SCALE, 239 - 24 * THERMAL_SCALE, 10, 24 * THERMAL_SCALE, gradientFb);

		//buttons description
		display->printf(290, 140, YELLOW, backgroundColor, "R"); //up
		display->printf(272, 125, YELLOW, backgroundColor, " F"); //left
		display->printf(290, 125, YELLOW, backgroundColor, "S"); //center
		display->printf(303, 125, YELLOW, backgroundColor, "[]"); //right
		display->printf(290, 110, YELLOW, backgroundColor, "M"); //down

		isStaticPartsRendered = true;
	}

	//info content
	if (delayCntr >= DRAW_DELAY) {
		const int16_t maxTemp = (int16_t)irSensor->getMaxTemp();
		const int16_t minTemp = (int16_t)irSensor->getMinTemp();

		//right panel
		display->fillScreen(235, 225, 235 + 40, 225 + 14, backgroundColor); 
		display->printf(235, 225, WHITE, backgroundColor, "%d\x81", maxTemp);
		display->fillScreen(235, 68, 235 + 40, 68 + 14, BLACK);
		display->printf(235, 68, GREEN, backgroundColor, "%d\x81", minTemp);

		//status strings
		if (strlen(statusLine) > 0)
		{
			display->printf(0, 28, WHITE, backgroundColor, "%s", statusLine);
		}
		const uint16_t cpuUsage = osGetCPUUsage();
		display->printf(0, 14, WHITE, backgroundColor, "E=%.2f Rate=%s", opts->emission, sensorRateToString(opts->sensorRefreshRate));
		display->printf(0, 0, WHITE, backgroundColor, "CPU:%02u%%", cpuUsage);

		delayCntr = 0;
	}
	delayCntr++;

	//runtime content
	irSensor->VisualizeImage(framebuffer, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, 1); //prepare thermal image in framebuffer
	//draw central mark with temp in framebuffer
	if (opts->showCenterTempMarker) {
		display->drawMarkInBuf(framebuffer, 24 * THERMAL_SCALE, centerX, centerY, WHITE);
		display->printf(framebuffer, 24 * THERMAL_SCALE, centerX - 18, centerY - 25, WHITE, "%.1f\x81", irSensor->getCenterTemp());
	}

	//thermal image famebuffer
	display->bufferDraw(0, 239 - 24 * THERMAL_SCALE, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, framebuffer);
}

void UI::DrawSettingsScreen()
{
	if (!isStaticPartsRendered)
	{
		display->clear(backgroundColor);
		display->printf(10, 225, WHITE, backgroundColor, "SETTINGS");

		isStaticPartsRendered = true;
	}

	//info content
	if (delayCntr >= DRAW_DELAY) {
		const uint16_t cpuUsage = osGetCPUUsage();
		display->printf(0, 0, WHITE, backgroundColor, "CPU:%02u%%", cpuUsage);

		delayCntr = 0;
	}
	delayCntr++;

	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	GetDateTime(&date, &time);

	const StoredOptionsDef_t* opts = options->GetCurrent();

	for (uint8_t i = 0; i < MENU_ITEMS_COUNT; i++)
	{
		const uint16_t lineStartY = 210 - 14 * (i + 1);
		const uint16_t fCol = GetMenuFrontColor(menuItems[i].id, selectedItemIndex, true);
		const uint16_t bCol = GetMenuBackColor(menuItems[i].id, selectedItemIndex, true);
		
		display->printf(10, lineStartY, fCol, bCol, "%s", menuItems[i].name);

		//draw submenu
		const uint16_t shift = strlen(menuItems[i].name) * 8 + 20;
		const bool isActualMenuItemInEdit = (menuItems[i].id == selectedItemIndex) && isMenuItemInEdit;
		if (menuItems[i].id == (int)MenuItems::DATE)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%02u", date.Date);
			display->printf(shift + 2 * 8, lineStartY, WHITE, backgroundColor, "-");
			DrawSubItem(shift + 3 * 8, lineStartY, 1, isActualMenuItemInEdit, "%02u", date.Month);
			display->printf(shift + 5 * 8, lineStartY, WHITE, backgroundColor, "-");
			DrawSubItem(shift + 6 * 8, lineStartY, 2, isActualMenuItemInEdit, "20%02u", date.Year);
		}
		else if (menuItems[i].id == (int)MenuItems::TIME)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "%02u", time.Hours);
			display->printf(shift + 2 * 8, lineStartY, WHITE, backgroundColor, ":");
			DrawSubItem(shift + 3 * 8, lineStartY, 1, isActualMenuItemInEdit, "%02u", time.Minutes);
			display->printf(shift + 5 * 8, lineStartY, WHITE, backgroundColor, ":");
			DrawSubItem(shift + 6 * 8, lineStartY, 2, isActualMenuItemInEdit, "%02u", time.Seconds);	
		}
		else if (menuItems[i].id == (int)MenuItems::EMISSION)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%1.2f]", opts->emission);
		}
		else if (menuItems[i].id == (int)MenuItems::SENSOR_RATE)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", sensorRateToString(opts->sensorRefreshRate));
		}
		else if (menuItems[i].id == (int)MenuItems::SENSOR_ADC_RESOLUTION)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", sensorAdcResolutionToString(opts->sensorAdcResolution));
		}
		else if (menuItems[i].id == (int)MenuItems::COLOR_SCHEME)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", colorSchemeToString(opts->colorScheme));
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_MIN_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", opts->showMinTempMarker ? "Y" : "N");
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_MAX_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", opts->showMaxTempMarker ? "Y" : "N");
		}
		else if (menuItems[i].id == (int)MenuItems::SHOW_CENTER_TEMP_MARK)
		{
			DrawSubItem(shift, lineStartY, 0, isActualMenuItemInEdit, "[%s]", opts->showCenterTempMarker ? "Y" : "N");
		}
	}
}

void UI::DrawFilesListScreen()
{
	if (!isStaticPartsRendered)
	{
		display->clear(backgroundColor);
		display->printf(10, 225, WHITE, backgroundColor, "SAVED FILES");
	}

	if ((delayCntr == DRAW_DELAY) && (filesCount > 0)) {

		const uint16_t pageNumber = selectedItemIndex / MAX_FILES_ON_SCREEN;
		const uint16_t pagesCount = filesCount / MAX_FILES_ON_SCREEN + 1;
		const uint32_t skipItems = pageNumber * MAX_FILES_ON_SCREEN;

		if ((pageNumber == pagesCount - 1) && (pagesCount > 1))
		{
			display->fillScreen(10, 0, 200, 225, backgroundColor); //clear list area
		}

		//read files list
		DIR dir;
		if (sdCard->OpenDir(&dir, "/"))
		{
			uint8_t i = 0;
			uint32_t skipCnt = 0;
			FILINFO file;
			while(true)
			{
				if(!sdCard->ReadDir(&dir, &file))
				{
					break;
				}
				if (file.fname[0] == 0)
				{
					break;
				}
				if (!(file.fattrib & AM_DIR)) //if not directory entry
				{
					if (skipCnt < skipItems)
					{
						skipCnt++;
						continue;
					}
					const uint16_t lineStartY = 210 - 14 * (i + 1);
					const uint16_t fCol = GetMenuFrontColor(i + pageNumber * MAX_FILES_ON_SCREEN, selectedItemIndex, true);
					const uint16_t bCol = GetMenuBackColor(i + pageNumber * MAX_FILES_ON_SCREEN, selectedItemIndex, true);

					display->printf(10, lineStartY, fCol, bCol, "%s", file.fname);
					i++;

					if (i >= MAX_FILES_ON_SCREEN)
					{
						break;
					}
				}
			}
			sdCard->CloseDir(&dir);
		}

		delayCntr = 0;
	}

	if (!isStaticPartsRendered)
	{
		delayCntr = DRAW_DELAY;
		isStaticPartsRendered = true;
	}
}

void UI::DrawFileViewScreen()
{
	if (!isStaticPartsRendered)
	{
		display->clear(backgroundColor);
		display->printf(10, 225, WHITE, backgroundColor, "FILE %u: %s", selectedItemIndex + 1, selectedFileName);

		memset(dots, 0x00, sizeof(dots));
		if (sdCard->ReadThvFile(selectedFileName, dots))
		{
			display->bufferDraw(10 + 32 * THERMAL_SCALE, 45, 10, 24 * THERMAL_SCALE, gradientFb);

			irSensor->FindMinAndMaxTemp();
			display->printf(245, 200, WHITE, backgroundColor, "%d\x81", (uint16_t)irSensor->getMaxTemp());
			display->printf(245, 40, GREEN, backgroundColor, "%d\x81", (uint16_t)irSensor->getMinTemp());

			irSensor->VisualizeImage(framebuffer, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, 1); //prepare thermal image in framebuffer
			if (options->GetCurrent()->showCenterTempMarker) {
				const uint16_t centerX = 32 / 2 * THERMAL_SCALE;
				const uint16_t centerY = 24 / 2 * THERMAL_SCALE;
				display->drawMarkInBuf(framebuffer, 24 * THERMAL_SCALE, centerX, centerY, WHITE);
				display->printf(framebuffer, 24 * THERMAL_SCALE, centerX - 18, centerY - 25, WHITE, "%.1f\x81", irSensor->getCenterTemp());
			}

			//thermal image famebuffer
			display->bufferDraw(10, 45, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, framebuffer);
		}

		isStaticPartsRendered = true;
	}
}

void UI::DrawUsbConnectedScreen()
{
	if (!isStaticPartsRendered)
	{
		display->clear(backgroundColor);
		display->printf(80, 120, YELLOW, backgroundColor, "USB STORAGE MODE");

		isStaticPartsRendered = true;
	}
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
	return isActive && (menuIndex == activeMenuIndex) ? backgroundColor : WHITE;
}

uint16_t UI::GetMenuBackColor(int8_t menuIndex, int8_t activeMenuIndex, bool isActive)
{
	return isActive && (menuIndex == activeMenuIndex) ? WHITE : backgroundColor;
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

void UI::setIsCharging(bool isCharging)
{
	this->isCharging = isCharging;
}
