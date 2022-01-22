/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "main.h"
#include "hardware.h"
#include "i2c.h"
#include "thermal.h"
#include "ili9341.h"
#include <string.h>

osThreadId LEDThread1Handle, LEDThread2Handle, IRSensorThreadHandle, ReadKeysTaskHandle, DrawTaskHandle;

IRSensor irSensor;
ILI9341 display;

volatile uint8_t vis_mode = 1;
volatile bool isSensorReady = false;
volatile bool isSensorReadDone = false;
volatile bool isFrameReady = false;
volatile TickType_t xSensorTime = 0;
volatile TickType_t xDrawTime = 0;
volatile uint32_t inWait = 0;

static void LED_Thread1(void const *argument);
static void LED_Thread2(void const *argument);
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

	display.init(&spi1);
	display.clear(BLACK);
	
	isSensorReady = irSensor.init(&i2c1, ALTERNATE_COLOR_SCHEME);

	osThreadDef(LED1, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(LED2, LED_Thread2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(READ_KEYS, ReadKeys_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(IR_SENSOR, IrSensor_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE + 1024);
	osThreadDef(DRAW_TASK, DrawTask_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE + 1024);
  
	LEDThread1Handle = osThreadCreate(osThread(LED1), NULL);
	LEDThread2Handle = osThreadCreate(osThread(LED2), NULL);
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

static void LED_Thread1(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		// GPIO_WritePin(USR_LED_PORT, USR_LED1_PIN, isSensorReadDone);
		osDelay(500);
	}
}

static void LED_Thread2(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 1);
		osDelay(500);
		GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 0);
		osDelay(500);
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
				osDelay(5);
			}
			irSensor.readImage(0.95f); //first subpage
			while (!irSensor.isFrameReady())
			{
				inWait = inWait + 1;
				osDelay(5);
			}
			irSensor.readImage(0.95f); // second subpage
			irSensor.findMinAndMaxTemp();
			const TickType_t xTime2 = xTaskGetTickCount();
			xSensorTime = xTime2 - xTime1;
			isSensorReadDone = true;
		}
		osDelay(10);
	}
}

static void DrawTask_Thread(void const *argument)
{
	(void) argument;
	for (;;)
	{
		const TickType_t xTime1 = xTaskGetTickCount();
		//display.clear(BLUE);

		display.fillScreen(0, 16, 319, 239, xTime1);
		display.printf(0, 0, WHITE, BLACK, "Frame:%04ums   Scan:%04ums", xDrawTime, xSensorTime);
		//display.drawBorder(0, 0, 319, 239, 1, WHITE);
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
		if (GPIO_ReadPin(USR_BTN_PORT, USR_BTN_PIN))
		{
			if (isPressed)
			{
				osDelay(350);
				continue;
			}
			vis_mode++;
			if (vis_mode > 2)
			{
				vis_mode = 0;
			}

			isPressed = true;
		}
		else
		{
			isPressed = false;
		}
		osDelay(75);
	}
}

void Error_Handler(const uint8_t source)
{
	uint8_t k = source;
	GPIO_WritePin(USR_LED2_PORT, USR_LED2_PIN, 1);
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
