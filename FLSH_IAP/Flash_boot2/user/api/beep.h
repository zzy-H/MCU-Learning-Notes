#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"


#define BeepPort			GPIOC
#define BeepPin				GPIO_Pin_0
#define BeepClk				RCC_APB2Periph_GPIOC

#define BeepOn()			(GPIO_SetBits(BeepPort, BeepPin))
#define BeepOff()			(GPIO_ResetBits(BeepPort, BeepPin))
#define BeepToggle()	(BeepPort->ODR ^= BeepPin)

void Beep_Config(void);
#endif
