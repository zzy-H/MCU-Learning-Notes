#ifndef __APP_H
#define __APP_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "led.h"
#include "key.h"
#include "beep.h"
#include "usart.h"
#include "stdio.h"
#include "my1680.h"
#include "esp8266.h"
#include "dht11.h"
#include "rgb.h"
#include "adc.h"
#include "time.h"
#include "rtc.h"
#include "bsp_lcd.h"
#include "spi_flash.h"
#include "math.h"

#include "lvgl.h"
#include "lv_port_disp.h"

void Bsp_Init(void);


extern TaskHandle_t xStartTaskHandle;
void vTaskStartCode( void * pvParameters );




#endif
