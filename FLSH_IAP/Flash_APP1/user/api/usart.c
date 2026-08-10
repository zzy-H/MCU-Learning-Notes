#include "usart.h"
#include "stdio.h"
#include "stmflash.h"
#include "delay.h"


uint8_t recvBuff[2] = {0};
uint32_t recvNum = 0;
uint32_t addr = FLASH_APP2_ADDR;
uint8_t recvTime = 0;
uint8_t recvOver = 0;
uint16_t app2FlagBuff[2] = {0xAAAA, 0xAAAA};

void RecvTimeOut(void)	//1ms一次
{
	if(recvTime) {
		recvTime++;
		if(recvTime >= 100) {
			recvOver = 1;
			recvTime = 0;
		}
	}
}

void RecvOverFun(void)
{
	if(recvOver == 1) {
		printf("APP数据接收完成:%d\r\n", recvNum);
		recvNum = 0;
		recvOver = 0;
		recvTime = 0;
		addr = FLASH_APP2_ADDR;
		
		STMFLASH_WriteHalfWord(APP2_FLAG_ADDR, app2FlagBuff[0]);
		STMFLASH_WriteHalfWord(APP2_FLAG_ADDR+2, app2FlagBuff[0]);
		printf("核对数据无误后，请按下复位按键进行数据更新\r\n");
	}
}


void USART1_IRQHandler(void)
{
	uint8_t data = 0;
	uint16_t temp = 0;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
		data = USART1->DR;//读取收到的数据
		recvBuff[recvNum%2] = data;

		if((recvNum%2) == 1) {
			temp = ((u16)recvBuff[1]<<8) + ((u16)recvBuff[0]); 
			STMFLASH_WriteHalfWord(addr, temp);
			addr+=2;
		}
		recvNum++;
		recvTime = 1;
	}
}






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
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//打开USART1的RXNE中断
	
	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_EnableIRQ(USART1_IRQn);
	
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

