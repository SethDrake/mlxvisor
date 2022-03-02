#include "ui.h"
#include <string.h>
#include "cpu_utils.h"

UI::UI()
{
	this->currentSreen = UIScreen::MAIN;
	this->isStaticPartsRendered = false;
	this->display = NULL;
	this->irSensor = NULL;
	this->options = NULL;
	this->delayCntr = DRAW_DELAY;

	memset(framebuffer, 0x10, sizeof(framebuffer));
	memset(gradientFb, 0x10, sizeof(gradientFb));
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
	this->currentSreen = screen;
	this->isStaticPartsRendered = false;
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
			mlx90640_refreshrate_t rate = opts->sensorRefreshRate;
			if (rate == MLX90640_64_HZ)
			{
				rate = MLX90640_0_5_HZ;
			}
			else
			{
				rate = (mlx90640_refreshrate_t)((int)rate + 1);
			}
			opts->sensorRefreshRate = rate;
			options->SaveOptions();
			irSensor->Reset();
			irSensor->setRefreshRate(rate);
		}
		else if (isButtonPressed(Button::LEFT))
		{
			float emission = opts->emission;
			if (emission > 0.1)
			{
				emission -= 0.05f;
			}
			opts->emission = emission;
			options->SaveOptions();
		}
		else if (isButtonPressed(Button::RIGHT))
		{
			float emission = opts->emission;
			if (emission < 1.0)
			{
				emission += 0.05f;
			}
			opts->emission = emission;
			options->SaveOptions();
		}
		else if (isButtonPressed(Button::DOWN))
		{
			//setScreen(UIScreen::SETTINGS);
		}
		else if (isButtonPressed(Button::OK))
		{
			
		}

		if (isAnyButtonPressed())
		{
			delayCntr = DRAW_DELAY;
			buttonsState = 0;
		}
	}
}

void UI::DrawBattery()
{
	if (adcVbat > 0) {
		const float vBat = 0.3f + 2 * (3.3f * adcVbat) / 4096;
		display->printf(273, 225, WHITE, BLACK, "[%1.2f]", vBat);
	}
}

void UI::DrawClock()
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	HAL_RTC_GetTime(&rtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc, &date, RTC_FORMAT_BIN);

	display->printf(240, 14, WHITE, BLACK, "%02u-%02u-20%02u", date.Date, date.Month, date.Year);
	display->printf(248, 0, WHITE, BLACK, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
}

void UI::DrawScreen()
{
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
		case UIScreen::CONFIRM: 
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
	const uint16_t centerX = (32 - 1) / 2 * THERMAL_SCALE;
	const uint16_t centerY = (24 - 1) / 2 * THERMAL_SCALE;
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
		display->printf(260, 150, WHITE, BLACK, "RATE"); //up
		display->printf(240, 130, WHITE, BLACK, "E-"); //left
		display->printf(260, 130, WHITE, BLACK, "SAVE"); //center
		display->printf(300, 130, WHITE, BLACK, "E+"); //right
		display->printf(260, 110, WHITE, BLACK, "MENU"); //down

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

const char* UI::sensorRateToString(mlx90640_refreshrate_t rate)
{
	switch(rate)
	{
		case MLX90640_0_5_HZ: return "0.5Hz";
		case MLX90640_1_HZ: return "1Hz  ";
		case MLX90640_2_HZ: return "2Hz  ";
		case MLX90640_4_HZ: return "4Hz  ";
		case MLX90640_8_HZ: return "8Hz  ";
		case MLX90640_16_HZ: return "16Hz ";
		case MLX90640_32_HZ: return "32Hz ";
		case MLX90640_64_HZ: return "64Hz ";
	}
	return "";
}
