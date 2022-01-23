/* Includes ------------------------------------------------------------------*/
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "main.h"
#include "thermal.h"
#include "ili9341.h"
#include <string.h>

#include "cpu_utils.h"

uint16_t framebuffer[24 * THERMAL_SCALE * 32 * THERMAL_SCALE];
uint16_t gradientFb[10 * 24 * THERMAL_SCALE];

osThreadId LEDThread1Handle, LEDThread2Handle, IRSensorThreadHandle, ReadKeysTaskHandle, DrawTaskHandle;

IRSensor irSensor;
ILI9341 display;
float emissivity = 0.95f;

volatile uint8_t vis_mode = 1;
volatile bool isSensorReady = false;
volatile bool isSensorReadDone = false;
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
	DMA_Init();

	display.init(&spi1);
	display.clear(BLACK);
	memset(framebuffer, 0x10, sizeof(framebuffer));
	memset(gradientFb, 0x10, sizeof(gradientFb));
	
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
		if (isSensorReady)
		{
			GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 1);
			osDelay(250);
		}
		GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 0);
		osDelay(250);
	}
}

static void LED_Thread2(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		/*GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 1);
		osDelay(1000);
		GPIO_WritePin(USR_LED1_PORT, USR_LED1_PIN, 0);*/
		osDelay(1000);
	}
}

static void IrSensor_Thread(void const *argument)
{
	(void) argument;
	uint8_t oneTimeOpDone = 0;

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
			irSensor.calculateTempMap(emissivity);
			irSensor.findMinAndMaxTemp();
			irSensor.visualizeImage(framebuffer, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, vis_mode);

			if (!oneTimeOpDone) {
				irSensor.drawGradient(gradientFb, 10, 24 * THERMAL_SCALE);
				oneTimeOpDone = 1;
			}

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
	uint8_t maxi = 4;
	uint8_t oneTimeOpDone = 0;

	for (;;)
	{
		if (!oneTimeOpDone) {
			display.bufferDraw(224, 71, 10, 24 * THERMAL_SCALE, gradientFb);
			oneTimeOpDone = 1;
		}

		const uint16_t cpuUsage = osGetCPUUsage();
		const TickType_t xTime1 = xTaskGetTickCount();
		display.bufferDraw(0, 71, 32 * THERMAL_SCALE, 24 * THERMAL_SCALE, framebuffer);
		if (i >= maxi) {
			display.printf(0, 0, WHITE, BLACK, "Frm:%03ums Scan:%03ums CPU:%02u%% VM:%01u W:%02u", xDrawTime, xSensorTime, cpuUsage, vis_mode, inWait);
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
		if (GPIO_ReadPin(USR_BTN_PORT, USR_BTN_PIN))
		{
			if (isPressed)
			{
				osDelay(350);
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
