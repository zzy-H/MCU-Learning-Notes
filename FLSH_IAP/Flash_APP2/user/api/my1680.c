#include "my1680.h"


__MY1680_DATA_TypeDef my1680Data = {.head = 0x7E, .end = 0xEF};

//UART5	TX PC12
void Usart5_Config(void)
{
	//发送和接收引脚
	//发送复用推挽输出，接收浮空输入
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不开硬件控制流
	USART_InitStructure.USART_Mode = USART_Mode_Tx;//发送器和接收器使能
	USART_InitStructure.USART_Parity = USART_Parity_No;//没有奇偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8个数据位
	USART_Init(UART5, &USART_InitStructure);
	
	USART_Cmd(UART5, ENABLE);//使能
}


void Usart5_SendByte(uint8_t data)
{
	while(USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET) 
	{}
	USART_SendData(UART5, data);	
}

void Usart5_SendBuff(uint8_t * buff, uint32_t len)
{
	for(uint32_t i=0; i<len; i++) {
		Usart5_SendByte(buff[i]);
	}
}


/**
	* @brief  my1680 根据指令以及参数整合要发送的整体数据
  * @param  myData
  * @retval 
  */
void My1680_SendCMDData(__MY1680_DATA_TypeDef myData)
{
	uint8_t i=0;
	uint8_t my1680SendBuff[7] = {0};
	my1680SendBuff[i++] = myData.head;
	my1680SendBuff[i++] = 1+1+myData.paramLen+1;
	my1680SendBuff[i++] = myData.cmd;
	myData.check = my1680SendBuff[1]^my1680SendBuff[2];
	for(uint8_t j=0; j<myData.paramLen; j++) {
		my1680SendBuff[i++] = myData.param[j];
		myData.check ^= myData.param[j];
	}
	my1680SendBuff[i++] = myData.check;
	my1680SendBuff[i++] = myData.end;
	
	Usart5_SendBuff(my1680SendBuff, i);
}


void My1680_Init(void)
{
	Usart5_Config();
	My1680_SetVoiceSize(30);
}

/**
	* @brief  my1680 设置音量大小
	* @param  size：音量大小 -- 0-30
  * @retval 
  */
void My1680_SetVoiceSize(uint8_t size)
{
	my1680Data.cmd = 0x31;
	my1680Data.param[0] = size;
	my1680Data.paramLen = 1;
	My1680_SendCMDData(my1680Data);
}


/**
	* @brief  my1680 播报制定文件夹下的某个曲目
	* @param  dirNum:文件夹序号  fileNum:文件序号
  * @retval 
  */
void My1680_PlayDirFile(uint8_t dirNum, uint8_t fileNum)
{
	my1680Data.cmd = MY1680_PLAY_DIRFILE_CMD;
	my1680Data.param[0] = dirNum;
	my1680Data.param[1] = fileNum;
	my1680Data.paramLen = 2;
	My1680_SendCMDData(my1680Data);
}



//假如说我们的0-10音频在 00 文件夹下
void My1680_PlayNum(uint8_t num)
{
	if(num > 99)	num = 99;
	uint8_t shi = num / 10;
	uint8_t ge = num %10;
	if(shi == 1){
		My1680_PlayDirFile(0, 10);
	}
	else if(shi > 1) {
		My1680_PlayDirFile(0, shi);
		My1680_PlayDirFile(0, 10);
	}
	if(ge != 0){
		My1680_PlayDirFile(0, ge);
	}
	if(num == 0) My1680_PlayDirFile(0, 0);
}




