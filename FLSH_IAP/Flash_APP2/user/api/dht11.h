#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

#define DHT11_OUT	1
#define DHT11_IN	0

#define DHT11_OUT_HIGH()	(GPIO_SetBits(GPIOG, GPIO_Pin_11))
#define DHT11_OUT_LOW()		(GPIO_ResetBits(GPIOG, GPIO_Pin_11))

#define DHT11_READ_IO()		(GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_11))

typedef struct{
	uint8_t Hum;
	float Tem;
}__DHT11_VALUE_TypeDef;




uint8_t Dht11_ReadData(__DHT11_VALUE_TypeDef * value);
#endif
