#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "stm32f10x.h"  

#define STM32_FLASH_SIZE 512 	 		//所选STM32的FLASH容量大小(单位为K)

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif	




//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
 
u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据

void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)	;
	
void STMFLASH_WriteHalfWord(u32 startAddr, u16 data);
uint8_t STMFLASH_WriteEasy(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)	;
uint8_t STMFLASH_Erase(u32 startAddr, u32 eraseSize) ;
#endif

















