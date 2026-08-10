#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"


//我们使用的FLASH大小：512K
//0x08000000-0x08080000
//Boot -- 12K			0x08000000-0x08002FFF		0x3000
//APP1 -- 250K		0x08003000-0x080417FF		0x3E800
//APP2 -- 250K		0x08041800-0x08080000		0x3E800
#define APP_MAX_SIZE			0x3E800
#define FLASH_APP1_ADDR		0x08003000  		//第一个应用程序起始地址(存放在FLASH)
#define FLASH_APP2_ADDR		0x08041800			//第二个应用程序起始地址(存放在FLASH)
#define APP2_FLAG_ADDR		(0x08080000-4)	//是否有更新程序标志位


#define STM32_FLASH_SIZE 512 	 		//所选STM32的FLASH容量大小(单位为K)

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif	


//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址

 






void Usart1_Config(void);
void Usart1_SendByte(uint8_t data);
void Usart1_SendBuff(uint8_t * buff, uint32_t len);
uint8_t Usart1_RecvByte(void);
#endif
