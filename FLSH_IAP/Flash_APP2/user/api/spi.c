#include "spi.h"

//W25Q			SPI2	
//MOSI		复用推挽输出		PB15
//MISO		浮空输入				PB14
//SCL			复用推挽输出		PB13		
//CS			通用推挽输出		PB12
void Spi2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

  /*!< sFLASH_SPI_CS_GPIO, sFLASH_SPI_MOSI_GPIO, sFLASH_SPI_MISO_GPIO 
       and sFLASH_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(sFLASH_CS_GPIO_CLK | sFLASH_SPI_MOSI_GPIO_CLK | sFLASH_SPI_MISO_GPIO_CLK |
                         sFLASH_SPI_SCK_GPIO_CLK, ENABLE);

  /*!< sFLASH_SPI Periph clock enable */
  sFLASH_SPI_APB_CMD(sFLASH_SPI_CLK, ENABLE);
  
  /*!< Configure sFLASH_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sFLASH_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MOSI_PIN;
  GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sFLASH_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
  GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  /*!< Configure sFLASH_CS_PIN pin: sFLASH Card CS pin */
  GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);
	
	
	//初始化SPI2		SP0/3模式都可以 高位先发  8bit
	SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(sFLASH_SPI, &SPI_InitStructure);
  SPI_Cmd(sFLASH_SPI, ENABLE);
}



uint8_t Spi2_SendRecvData(uint8_t data)
{
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(sFLASH_SPI, data);
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(sFLASH_SPI);
}


//void sFLASH_WriteEnable(void)
//{
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x06);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//}

//void sFLASH_WriteDisable(void)
//{
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x04);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//}

//uint8_t sFLASH_ReadStatue(void)
//{
//	uint8_t state = 0;
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x05);
//	state = Spi2_SendRecvData(0x00);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	return state;
//}


//void sFLASH_ReadData(uint32_t addr, uint8_t * buff, uint32_t len)
//{
//	uint8_t data = 0;
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x03);
//	Spi2_SendRecvData((addr&0xFF0000)>>16);
//	Spi2_SendRecvData((addr&0x00FF00)>>8);
//	Spi2_SendRecvData((addr&0x0000FF));
//	for(uint32_t i=0; i<len;	i++)
//		buff[i] = Spi2_SendRecvData(0x00);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//}

//void sFLASH_SectorErase(void)
//{
//	
//	sFLASH_WriteEnable();
//	while((sFLASH_ReadStatue()&0x01) != 0)
//	{}
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x20);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	while((sFLASH_ReadStatue()&0x01) != 0)
//	{}
//}

//void sFLASH_PageProgram(uint32_t addr, uint8_t * buff, uint32_t len)
//{
//	sFLASH_WriteEnable();
//	while((sFLASH_ReadStatue()&0x01) != 0)
//	{}
//	GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//	Spi2_SendRecvData(0x02);
//	Spi2_SendRecvData((addr&0xFF0000)>>16);
//	Spi2_SendRecvData((addr&0x00FF00)>>8);
//	Spi2_SendRecvData((addr&0x0000FF));
//	for(uint32_t i=0; i<len;	i++)
//		Spi2_SendRecvData(buff[i]);
//	GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN);
//}


