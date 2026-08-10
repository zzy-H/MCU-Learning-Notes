#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"


#define Led1Port	GPIOE
#define Led1Pin		GPIO_Pin_2
#define Led1Clk		RCC_APB2Periph_GPIOE

#define Led2Port	GPIOE
#define Led2Pin		GPIO_Pin_3
#define Led2Clk		RCC_APB2Periph_GPIOE

#define Led3Port	GPIOE
#define Led3Pin		GPIO_Pin_4
#define Led3Clk		RCC_APB2Periph_GPIOE

#define Led4Port	GPIOE
#define Led4Pin		GPIO_Pin_5
#define Led4Clk		RCC_APB2Periph_GPIOE


void Led_Config(void);
void LedOn(GPIO_TypeDef * port, uint16_t pin);
void LedOff(GPIO_TypeDef * port, uint16_t pin);
void LedToggle(GPIO_TypeDef * port, uint16_t pin);

void LedTest(uint8_t ledFlag);
#endif
