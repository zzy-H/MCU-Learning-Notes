#include "dht11.h"
#include "delay.h"

//DHT11	PG11
void Dht11_SetIOMode(uint8_t mode)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
//	if(mode == DHT11_OUT)
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	else if(mode == DHT11_IN)
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}


uint8_t Dht11_ReadData(__DHT11_VALUE_TypeDef * value)
{
	uint8_t cnt = 0;
	uint8_t buff[5] = {0};
	Dht11_SetIOMode(DHT11_OUT);
	DHT11_OUT_HIGH();
	
	DHT11_OUT_LOW();
	Delay_ms(18);
	DHT11_OUT_HIGH();
	Dht11_SetIOMode(DHT11_IN);
	
	while(DHT11_READ_IO() == Bit_SET) {
		cnt++;
		if(cnt > 100)	return 1;//未响应
		Delay_us(1);
	}
	cnt = 0;
	while(DHT11_READ_IO() == Bit_RESET) {
		cnt++;
		if(cnt > 100)	return 2;//响应无法通过
		Delay_us(1);
	}	
	
	for(uint8_t i=0; i<40; i++) {
		cnt = 0;
		while(DHT11_READ_IO() == Bit_SET) {
			cnt++;
			if(cnt > 100)	return 3;//数据内没有低电平到来
			Delay_us(1);
		}
		cnt = 0;
		while(DHT11_READ_IO() == Bit_RESET) {
			cnt++;
			if(cnt > 100)	return 4;//数据内没有高电平到来
			Delay_us(1);
		}	
		Delay_us(30);
		if(DHT11_READ_IO() == Bit_RESET) {
			//数据0
			buff[i/8] &= ~(1<<(7-(i%8)));
		}
		else if(DHT11_READ_IO() == Bit_SET) {
			//数据1
			buff[i/8] |= (1<<(7-(i%8)));
		}	
	}
	Dht11_SetIOMode(DHT11_OUT);
	DHT11_OUT_HIGH();
	if(buff[4] != (buff[0]+buff[1]+buff[2]+buff[3])) {
		return 5;//校验失败
	}
	value->Hum = buff[0];
	value->Tem = buff[2]+((buff[3]&0x7F)/10.0);
	if((buff[3] & (1<<7)) != 0)	value->Tem = -value->Tem;
	return 0;
}










