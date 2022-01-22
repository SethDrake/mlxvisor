#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
	taskDISABLE_INTERRUPTS();
	Error_Handler(10);
}

void vApplicationMallocFailedHook(void) {
	taskDISABLE_INTERRUPTS();
	Error_Handler(11);
}
