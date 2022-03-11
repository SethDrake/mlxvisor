/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "main.h"
#include "hardware.h"
#include "thermal.h"
#include "ili9341.h"
#include "sdcard.h"
#include "ui.h"
#include "options.h"
#include <string.h>

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_storage.h"

osThreadId IRSensorThreadHandle, ReadKeysTaskHandle, DrawTaskHandle, BgTaskHandle;

Options options;
IRSensor irSensor;
ILI9341 display;
SDCard sdCard;
UI ui;

volatile uint8_t vis_mode = 1;
volatile bool isSensorReady = false;
volatile TickType_t xSensorTime = 0;
volatile uint32_t inWait = 0;

static void IrSensor_Thread(void const *argument);
static void ReadKeys_Thread(void const *argument);
static void DrawTask_Thread(void const *argument);
static void BgTask_Thread(void const *argument);

int main()
{
	HAL_Init();

	Clock_Init();
	GPIO_Init();
	I2C_Init();
	SPI_Init();
	ADC_Init();
	DMA_Init();

	options.LoadOptions();

	display.Init(&spi1);
	display.clear(BLACK);

	sdCard.Init();

	isSensorReady = irSensor.Init(&i2c1, options.GetCurrent()->sensorRefreshRate, options.GetCurrent()->sensorAdcResolution, options.GetCurrent()->colorScheme);

	ui.InitScreen(&display, &irSensor, &sdCard, &options);

	HAL_ADC_Start(&adc1);

	USB_Init();

	const osThreadDef_t os_thread_def_READ_KEYS = { (char*)"READ_KEYS", (ReadKeys_Thread), (osPriorityNormal), (0), (((uint16_t) 128 + 1024))};
	const osThreadDef_t os_thread_def_BG_TASK = { (char*)"BG_TASK", (BgTask_Thread), (osPriorityNormal), (0), (((uint16_t) 128))};
	const osThreadDef_t os_thread_def_IR_SENSOR = { (char*)"IR_SENSOR", (IrSensor_Thread), (osPriorityNormal), (0), (((uint16_t) 128) + 1024)};
	const osThreadDef_t os_thread_def_DRAW_TASK = { (char*)"DRAW_TASK", (DrawTask_Thread), (osPriorityNormal), (0), (((uint16_t) 128) + 2048)};

	ReadKeysTaskHandle = osThreadCreate(osThread(READ_KEYS), NULL);
	IRSensorThreadHandle = osThreadCreate(osThread(IR_SENSOR), NULL);
	DrawTaskHandle = osThreadCreate(osThread(DRAW_TASK), NULL);
	BgTaskHandle = osThreadCreate(osThread(BG_TASK), NULL);
  
	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	while (true)
	{
	}
}


static void IrSensor_Thread(void const *argument)
{
	(void) argument;

	for (;;)
	{
		if (isSensorReady && ui.isSensorReadActive()) {
			const TickType_t xTime1 = xTaskGetTickCount();
			while (!irSensor.isFrameReady())
			{
				inWait = inWait + 1;
				osDelay(10);
			}
			irSensor.ReadImage(); //first subpage
			while (!irSensor.isFrameReady())
			{
				inWait = inWait + 1;
				osDelay(10);
			}
			irSensor.ReadImage(); // second subpage

			irSensor.CalculateTempMap(options.GetCurrent()->emission);
			irSensor.FindMinAndMaxTemp();

			const TickType_t xTime2 = xTaskGetTickCount();
			xSensorTime = xTime2 - xTime1;
		}
		osDelay(20);
	}
}

static void DrawTask_Thread(void const *argument)
{
	(void) argument;
	
	for (;;)
	{
		ui.DrawScreen();
		osDelay(10);
	}
}

static void ReadKeys_Thread(void const *argument)
{
	(void) argument;

	for (;;)
	{
		ui.setButtonState(Button::UP, GPIO_ReadPin(USR_BTN_U_PORT, USR_BTN_U_PIN));
		ui.setButtonState(Button::RIGHT, GPIO_ReadPin(USR_BTN_R_PORT, USR_BTN_R_PIN));
		ui.setButtonState(Button::DOWN, GPIO_ReadPin(USR_BTN_D_PORT, USR_BTN_D_PIN));
		ui.setButtonState(Button::LEFT, GPIO_ReadPin(USR_BTN_L_PORT, USR_BTN_L_PIN));
		ui.setButtonState(Button::OK, GPIO_ReadPin(USR_BTN_OK_PORT, USR_BTN_OK_PIN));
		ui.ProcessButtons();

		osDelay(300);
	}
}

static void BgTask_Thread(void const *argument)
{
	(void) argument;

	for (;;)
	{
		ui.adcVbat = HAL_ADC_GetValue(&adc1);
		HAL_ADC_Start(&adc1);

		if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
		{
			if (ui.GetScreen() != UIScreen::USB_CONNECTED_MODE)
			{
				ui.setScreen(UIScreen::USB_CONNECTED_MODE);
			}
		}
		else
		{
			if (ui.GetScreen() == UIScreen::USB_CONNECTED_MODE)
			{
				ui.setScreen(UIScreen::MAIN);
			}
		}

		osDelay(1000);
	}
}

void Error_Handler(const uint8_t source)
{
	while (true) {}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
	while (1)
	{
	}
}
#endif
