#include "beep.h"


void Beep_Config(void)
{
	RCC_APB2PeriphClockCmd(BeepClk, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = BeepPin;
	GPIO_Init(BeepPort, &GPIO_InitStructure);
}





