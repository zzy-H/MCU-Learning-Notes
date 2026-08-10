#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

void Key_Config(void);
uint8_t Get_KeyValue(void);
void Key_ExtiConfig(void);
#endif
