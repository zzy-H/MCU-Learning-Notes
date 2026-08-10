#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

#define ESP8266_ENABLE()	(GPIO_SetBits(GPIOE, GPIO_Pin_6))
#define ESP8266_DISABLE()	(GPIO_ResetBits(GPIOE, GPIO_Pin_6))


#define ESP_RECV_MAX_SIZE	(2048)
typedef struct {
	uint16_t recvCnt;
	uint8_t recvBuff[ESP_RECV_MAX_SIZE];
}__ESP8266_DATATypedef;

extern __ESP8266_DATATypedef espData ;


void ESP8266_Init(void);
void Esp8266_SendString(char *str);
uint8_t Esp8266_SendATCMD(char * cmd, char * recv, uint32_t timeOut);
void Weather_GetDate(void);
#endif
