#include "key.h"
#include "delay.h"

extern uint8_t flag;

void Key_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
}


////非阻塞式的按键检测
////0：没有按下		1：短按			2：长按
//uint8_t Get_KeyValue(void)
//{
//	uint8_t retValue = 0;
//	static uint16_t keyCnt = 0;
//	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET) {
//		keyCnt++;
//	}
//	else if(keyCnt > 200) {
//		keyCnt = 0;
//		retValue = 2;
//	}
//	else if(keyCnt > 2) {
//		keyCnt = 0;
//		retValue = 1;
//	}
//	else {
//		keyCnt = 0;
//		retValue = 0;
//	}
//	return retValue;
//	
//}




// 0 没有按键按下
// 1 2 3 4 对应KEY1 2 3 4
uint8_t Get_KeyValue(void)
{
	uint8_t retValue = 0;
	
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET) {
		Delay_ms(10);//消抖检测
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET) {
			while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)//松手检测
			{}
			retValue = 1;
		}
	}
	
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET)  {
		Delay_ms(10);
		if((GPIOC->IDR & (1<<4)) == 0) {
			while((GPIOC->IDR & (1<<4)) == 0)
			{}
			retValue = 2;
		}
	}
	if((GPIOC->IDR & (1<<5)) == 0) {
		Delay_ms(10);
		if((GPIOC->IDR & (1<<5)) == 0) {
			while((GPIOC->IDR & (1<<5)) == 0)
			{}
			retValue = 3;
		}
	}
	if((GPIOC->IDR & (1<<6)) == 0) {
		Delay_ms(10);
		if((GPIOC->IDR & (1<<6)) == 0) {
			while((GPIOC->IDR & (1<<6)) == 0)
			{}
			retValue = 4;
		}
	}
	return retValue;
}








