#include "stmflash.h"
#include "delay.h"
#include "usart.h"
 

//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0; i<NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	  WriteAddr += 2;//地址增加2.
	}  
} 


//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead)   	
{
	u16 i;
	for(i=0; i<NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr += 2;//偏移2个字节.	
	}
}


//startAddr地址必须是2K的整数倍
//numToErase 要擦除的内存大小，必须是2K的整数倍
uint8_t STMFLASH_Erase(u32 startAddr, u32 eraseSize) 
{
	uint16_t i=0;
	uint16_t len = eraseSize/STM_SECTOR_SIZE;
	if(startAddr<STM32_FLASH_BASE||(startAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return -1;//非法地址
	if((startAddr % STM_SECTOR_SIZE))
		return -2;//非法地址
	FLASH_Unlock();//解锁
	for(i=0; i<len; i++) {
		FLASH_ErasePage(startAddr+i*STM_SECTOR_SIZE);//擦除这个扇区
	}
	FLASH_Lock();//上锁	
	return 0;
}

//半字数据写入
void STMFLASH_WriteHalfWord(u32 startAddr, u16 data)
{
	FLASH_Unlock();						//解锁
	FLASH_ProgramHalfWord(startAddr, data);
	FLASH_Lock();//上锁
}



















