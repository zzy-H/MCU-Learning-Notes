#include "app.h"

__DHT11_VALUE_TypeDef dht11 = {0};
__ADC_VALUE_TypeDef	adcValue = {0};

TaskHandle_t xSensorTaskHandle = NULL;
void vTaskSensorCode( void * pvParameters )
{
	configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
	for( ;; ) {
		taskENTER_CRITICAL();
		if(Dht11_ReadData(&dht11) == 0) {
			printf("Tem:%0.2f, Hum:%d\r\n", dht11.Tem, dht11.Hum);
		}
		taskEXIT_CRITICAL();
		Get_AdcValue(&adcValue);
		printf("light:%d, Air:%d\r\n", adcValue.Light, adcValue.MQ2);
		vTaskDelay(500);
	}
}

TaskHandle_t xWifiTaskHandle = NULL;
void vTaskWifiCode( void * pvParameters )
{
	configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
	for( ;; )
	{
		Weather_GetDate();
		vTaskDelay(1000);
	}
}


extern lv_obj_t * scr1;
extern lv_obj_t * scr2;
TaskHandle_t xKeyTaskHandle = NULL;
void vTaskKeyCode( void * pvParameters )
{
	uint8_t keyNum = 0;
	configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
	for( ;; )
	{
		switch(Get_KeyValue())
		{
			case 1:
				switch(keyNum)
				{
					case 0:lv_scr_load_anim(scr2, LV_SCR_LOAD_ANIM_OVER_LEFT, 2000, 0, false);	break;
					case 1:lv_scr_load_anim(scr1, LV_SCR_LOAD_ANIM_OVER_LEFT, 2000, 0, false);	break;
				}
				keyNum++;
				keyNum%=2;
				printf("key1\r\n");
			break;
			case 2:printf("key2\r\n");break;
			case 4:printf("key3\r\n");break;
			case 8:printf("key4\r\n");break;
		}
		vTaskDelay(10);
	}
}

TaskHandle_t xLvglTaskHandle = NULL;
void vLvglTaskFunction( void * pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS( 5 );
	xLastWakeTime = xTaskGetTickCount();  //获取系统时间
	for(;;) {		
		lv_task_handler();
		vTaskDelayUntil( &xLastWakeTime, xPeriod );//绝对延时
	}
}





TaskHandle_t xStartTaskHandle = NULL;
void vTaskStartCode( void * pvParameters )
{
	Bsp_Init();//硬件初始化
	taskENTER_CRITICAL();//进入临界区
	//创建一个FreeRTOS任务，用来轮询lvgl的任务
	xTaskCreate(vLvglTaskFunction, "lvgl_task", 512, NULL, 4, &xLvglTaskHandle);
	BaseType_t xReturnedLed = xTaskCreate( vTaskSensorCode,"sensor task",128,NULL,1,&xSensorTaskHandle);
	BaseType_t xReturnedWifi = xTaskCreate( vTaskWifiCode,"wifi task",256,NULL,1,&xWifiTaskHandle);
	BaseType_t xReturnedKey = xTaskCreate( vTaskKeyCode,"key task",128,NULL,3,&xKeyTaskHandle);
	taskEXIT_CRITICAL();
	configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
	for( ;; ) {
		RecvOverFun();
		LedToggle(Led1Port, Led1Pin);
		vTaskDelay(500);
	}
}















