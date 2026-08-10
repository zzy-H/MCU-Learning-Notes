#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

//栈区溢出钩子函数
void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName )
{
	UBaseType_t uxHighWaterMark;
	uxHighWaterMark = uxTaskGetStackHighWaterMark( xTask );
	printf ("%u : StackOverflow:%s(%lu)\r\n", xTaskGetTickCount(), pcTaskName, uxHighWaterMark);	
	while(1)
	{}
}

void vApplicationMallocFailedHook(void)
{
	printf ("Malloc Failed\r\n");	
	while(1)
	{}
}

#include "lvgl.h"
void vApplicationTickHook(void)
{
	lv_tick_inc(1); 
	RecvTimeOut();
	
}


