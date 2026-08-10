#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

typedef struct{
	uint16_t Light;
	uint16_t MQ2;
}__ADC_VALUE_TypeDef;




void Adc_Config(void);
void Get_AdcValue(__ADC_VALUE_TypeDef * adcValue);

#endif
