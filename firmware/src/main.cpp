/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "main.h"

#include <cmath>

#include "thermal.h"
#include "ili9341.h"
#include <string.h>

#include "cpu_utils.h"

static uint16_t framebuffer[24 * THERMAL_SCALE * 32 * THERMAL_SCALE];
static uint16_t gradientFb[10 * 24 * THERMAL_SCALE];

osThreadId IRSensorThreadHandle, ReadKeysTaskHandle, DrawTaskHandle;

IRSensor irSensor;
ILI9341 display;
float emissivity = 0.95f;

volatile uint8_t vis_mode = 1;
volatile bool isSensorReady = false;
volatile bool isSensorReadDone = false;
volatile bool isTImageDone = false;
volatile TickType_t xSensorTime = 0;
volatile TickType_t xDrawTime = 0;
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
	DMA_Init();

	display.init(&spi1);
	display.clear(BLACK);
	memset(framebuffer, 0x10, sizeof(framebuffer));
	memset(gradientFb, 0x10, sizeof(gradientFb));
	
	isSensorReady = irSensor.init(&i2c1, ALTERNATE_COLOR_SCHEME);

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

void drawRightPanel()
{
	const int16_t maxTemp = (int16_t)irSensor.getMaxTemp();
	const int16_t minTemp = (int16_t)irSensor.getMinTemp();

	display.fillScreen(235, 225, 235 + 40, 225 + 14, BLACK); //clear right panel
	display.printf(235, 225, WHITE, BLACK, "%d\x81", maxTemp);
	display.fillScreen(235, 68, 235 + 40, 68 + 14, BLACK);
	display.printf(235, 68, GREEN, BLACK, "%d\x81", minTemp);
}

void drawBottomPanel()
{
	const uint16_t cpuUsage = osGetCPUUsage();
	display.printf(0, 14, WHITE, BLACK, "E=%.2f", emissivity);
	display.printf(0, 0, WHITE, BLACK, "Frm:%03ums Scan:%03ums CPU:%02u%% VM:%01u W:%01u", xDrawTime, xSensorTime, cpuUsage, vis_mode, inWait);
}

static void IrSensor_Thread(void const *argument)
{
	(void) argument;
	const uint16_t centerX = (32-1)/2 * THERMAL_SCALE;
	const uint16_t centerY = (24-1)/2 * THERMAL_SCALE;
	uint8_t i = 0;
	const uint8_t maxi = 5;

	for (;;)
	{
		if (isSensorReady) {
			const TickType_t xTime1 = xTaskGetTickCount();
			while (!irSensor.isFrameReady())
			{
				inWait = inWait + 1;
				osDelay(10);
			}
			irSensor.readImage(); //first subpage
			while (!irSensor.isFrameReady())
			{
				inWait = inWait + 1;
				osDelay(10);
			}
			irSensor.readImage(); // second subpage
		    isTImageDone = false;
			irSensor.calculateTempMap(emissivity);

			if (i >= maxi) {
				irSensor.findMinAndMaxTemp();
				i = 0;
			}
			i++;

			irSensor.visualizeImage(framebuffer, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, vis_mode);
			//central mark with temp
			display.drawMarkInBuf(framebuffer, 24 * THERMAL_SCALE, centerX, centerY, WHITE);
			display.printf(framebuffer, 24 * THERMAL_SCALE, centerX - 18, centerY - 25, WHITE, "%.1f\x81", irSensor.getCenterTemp());
			isTImageDone = true;

			const TickType_t xTime2 = xTaskGetTickCount();
			xSensorTime = xTime2 - xTime1;
			isSensorReadDone = true;
		}
		osDelay(20);
	}
}

static void DrawTask_Thread(void const *argument)
{
	(void) argument;
	uint8_t i = 0;
	const uint8_t maxi = 25;
	uint8_t oneTimeOpDone = 0;
	
	for (;;)
	{
		if (!oneTimeOpDone) {
			irSensor.drawGradient(gradientFb, 10, 24 * THERMAL_SCALE);
			display.bufferDraw(32 * THERMAL_SCALE, 239 - 24 * THERMAL_SCALE, 10, 24 * THERMAL_SCALE, gradientFb);
			oneTimeOpDone = 1;
		}

		const TickType_t xTime1 = xTaskGetTickCount();

		if (isTImageDone) {
			display.bufferDraw(0, 239 - 24 * THERMAL_SCALE, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, framebuffer);
		}

		if (i >= maxi) {
			drawRightPanel();
			drawBottomPanel();
			i = 0;
		}
		i++;
		xDrawTime = xTaskGetTickCount() - xTime1;

		osDelay(20);
	}
}

static void ReadKeys_Thread(void const *argument)
{
	(void) argument;

	bool isPressed = false;
	for (;;)
	{
		if (GPIO_ReadPin(USR_BTN_OK_PORT, USR_BTN_OK_PIN))
		{
			if (isPressed)
			{
				osDelay(200);
				continue;
			}
			vis_mode++;
			if (vis_mode > 1)
			{
				vis_mode = 0;
			}

			isPressed = true;
		}
		else
		{
			isPressed = false;
		}
		osDelay(200);
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
