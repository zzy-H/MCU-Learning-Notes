#include "led.h"

/**
  * @brief  初始化led灯，LED1 2 3 4    PE2 3 4 5
	*					通用推挽输出最高速
  * @param  None
  * @retval None
  */

void Led_Config(void)
{
#ifdef USE_STDPERIPH_DRIVER
	//开GPIOE的时钟
	RCC_APB2PeriphClockCmd(Led1Clk | Led2Clk | Led3Clk | Led4Clk, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = Led1Pin;
	GPIO_Init(Led1Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = Led2Pin;
	GPIO_Init(Led2Port, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = Led3Pin;
	GPIO_Init(Led3Port, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = Led4Pin;
	GPIO_Init(Led4Port, &GPIO_InitStructure);	
#else
	//开PE时钟
	RCC->APB2ENR |= (1<<6);
	//配置成通用推挽输出最高速
	GPIOE->CRL &= ~(0xffff<<8);
	GPIOE->CRL |= (0x3333<<8);
#endif
	
	LedOff(Led1Port, Led1Pin);
	LedOff(Led2Port, Led2Pin);
	LedOff(Led3Port, Led3Pin);
	LedOff(Led4Port, Led4Pin);
}


/**
  * @brief  打开LED灯
  * @param  port：led灯的端口，pin：led灯的端口位 位移之后的值
  * @retval None
  */

void LedOn(GPIO_TypeDef * port, uint16_t pin)
{
#ifdef USE_STDPERIPH_DRIVER
	GPIO_ResetBits(port, pin);
#else
	port->ODR &= ~(pin);
#endif
}

/**
  * @brief  关闭LED灯
  * @param  port：led灯的端口，pin：led灯的端口位 位移之后的值
  * @retval None
  */

void LedOff(GPIO_TypeDef * port, uint16_t pin)
{
#ifdef USE_STDPERIPH_DRIVER
	GPIO_SetBits(port, pin);
#else
	port->ODR |= (pin);
#endif
}


/**
  * @brief  LED灯状态切换函数
  * @param  port：led灯的端口，pin：led灯的端口位 位移之后的值
  * @retval None
  */

void LedToggle(GPIO_TypeDef * port, uint16_t pin)
{
	port->ODR ^= (pin);
}



void LedTest(uint8_t ledFlag)
{
	static uint8_t ledCnt = 0;
	if(ledFlag == 1)
	{
		switch(ledCnt) {
			case 0:LedOn(Led1Port, Led1Pin);LedOff(Led4Port, Led4Pin);break;
			case 1:LedOn(Led2Port, Led2Pin);LedOff(Led1Port, Led1Pin);break;
			case 2:LedOn(Led3Port, Led3Pin);LedOff(Led2Port, Led2Pin);break;
			case 3:LedOn(Led4Port, Led4Pin);LedOff(Led3Port, Led3Pin);break;
		}
		ledCnt++;
		ledCnt %= 4;	
	}
	else {
		ledCnt = 0;
		LedOff(Led4Port, Led4Pin);
		LedOff(Led1Port, Led1Pin);
		LedOff(Led2Port, Led2Pin);
		LedOff(Led3Port, Led3Pin);
	}
}



