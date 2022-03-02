/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "main.h"
#include "thermal.h"
#include "ili9341.h"
#include "ui.h"
#include "options.h"
#include <cmath>
#include <string.h>



osThreadId IRSensorThreadHandle, ReadKeysTaskHandle, DrawTaskHandle;

Options options;
IRSensor irSensor;
ILI9341 display;
UI ui;

volatile uint8_t vis_mode = 1;
volatile bool isSensorReady = false;
volatile TickType_t xSensorTime = 0;
volatile uint32_t inWait = 0;

static void IrSensor_Thread(void const *argument);
static void ReadKeys_Thread(void const *argument);
static void DrawTask_Thread(void const *argument);

int main(void)
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

	isSensorReady = irSensor.Init(&i2c1, 
								  options.GetCurrent()->sensorRefreshRate, 
								  options.GetCurrent()->sensorAdcResolution, 
								  options.GetCurrent()->colorSchemeIndex == 0 ? DEFAULT_COLOR_SCHEME : ALTERNATE_COLOR_SCHEME);

	ui.InitScreen(&display, &irSensor, &options);

	osThreadDef(READ_KEYS, ReadKeys_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(IR_SENSOR, IrSensor_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE + 1024);
	osThreadDef(DRAW_TASK, DrawTask_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE + 1024);

	ReadKeysTaskHandle = osThreadCreate(osThread(READ_KEYS), NULL);
	IRSensorThreadHandle = osThreadCreate(osThread(IR_SENSOR), NULL);
	DrawTaskHandle = osThreadCreate(osThread(DRAW_TASK), NULL);
  
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
		if (isSensorReady) {
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

	bool isPressed = false;
	for (;;)
	{
		ui.setButtonState(Button::UP, GPIO_ReadPin(USR_BTN_U_PORT, USR_BTN_U_PIN));
		ui.setButtonState(Button::RIGHT, GPIO_ReadPin(USR_BTN_R_PORT, USR_BTN_R_PIN));
		ui.setButtonState(Button::DOWN, GPIO_ReadPin(USR_BTN_D_PORT, USR_BTN_D_PIN));
		ui.setButtonState(Button::LEFT, GPIO_ReadPin(USR_BTN_L_PORT, USR_BTN_L_PIN));
		ui.setButtonState(Button::OK, GPIO_ReadPin(USR_BTN_OK_PORT, USR_BTN_OK_PIN));
		ui.ProcessButtons();

		//HAL_ADC_Start_DMA(&adc1, (uint32_t*)&ui.adcVbat, 1);
		HAL_ADC_Start(&adc1);
		ui.adcVbat = HAL_ADC_GetValue(&adc1);
		osDelay(300);
	}
}

void Error_Handler(const uint8_t source)
{
	uint8_t k = source;
	//GPIO_WritePin(USR_LED3_PORT, USR_LED3_PIN, 1);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
