#include "delay.h"


void Delay_us(uint32_t t)
{
	while(t--) {
		delay_1us();
	}
}

void Delay_ms(uint32_t t)
{
	uint64_t T = t*1000;
	while(T--) {
		delay_1us();
	}
}












