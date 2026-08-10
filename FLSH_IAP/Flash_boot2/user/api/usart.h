#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

#define USART_REC_LEN  			(55*1024) //定义最大接收字节数 55K

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern u16 USART_RX_CNT;				//接收的字节数	 


void Usart1_Config(void);
void Usart1_SendByte(uint8_t data);
void Usart1_SendBuff(uint8_t * buff, uint32_t len);
uint8_t Usart1_RecvByte(void);
#endif
