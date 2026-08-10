#include "usart.h"
#include "stdio.h"



//USART1	TX PA9	RX PA10
void Usart1_Config(void)
{
	//发送和接收引脚
	//发送复用推挽输出，接收浮空输入
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不开硬件控制流
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//发送器和接收器使能
	USART_InitStructure.USART_Parity = USART_Parity_No;//没有奇偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8个数据位
	USART_Init(USART1, &USART_InitStructure);
	
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//打开USART1的RXNE中断
//	NVIC_SetPriority(USART1_IRQn, 1);
//	NVIC_EnableIRQ(USART1_IRQn);
	
	USART_Cmd(USART1, ENABLE);//使能USART1
}


void Usart1_SendByte(uint8_t data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) 
	{}
	USART_SendData(USART1, data);	
}

void Usart1_SendBuff(uint8_t * buff, uint32_t len)
{
	for(uint32_t i=0; i<len; i++) {
		Usart1_SendByte(buff[i]);
	}
}






uint8_t Usart1_RecvByte(void)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) 
	{}	
	return USART_ReceiveData(USART1);
}


//重新构造 fputc，可以使用printf函数
int fputc(int ch, FILE *f)
{
  Usart1_SendByte(ch);
  return ch;
}

