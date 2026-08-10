#ifndef __RGB_H
#define __RGB_H

#include "stm32f10x.h"

void RGB_Config(void);
void RGB_SetColor(uint16_t rPulse, uint16_t gPulse, uint16_t bPulse);
#endif
