#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "usart.h"
#include "stdio.h"
#include "stmflash.h"


//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}



typedef  void (*iapfun)(void);				//定义一个函数类型的参数.


//我们使用的FLASH大小：512K
//0x08000000-0x08080000
//Boot -- 12K			0x08000000-0x08002FFF		0x3000
//APP1 -- 250K		0x08003000-0x080417FF		0x3E800
//APP2 -- 250K		0x08041800-0x08080000		0x3E800
#define FLASH_APP1_ADDR		0x08003000  		//第一个应用程序APP1起始地址(存放在FLASH)
#define FLASH_APP2_ADDR		0x08041800      //第二个应用程序APP2的起始地址
#define APP1_FLAG_ADDR		(FLASH_APP2_ADDR-4)	//APP1是否有更新程序标志位
#define APP2_FLAG_ADDR		(0x08080000-4)	//APP2是否有更新程序标志位
#define APP_SIZE	(0x3E800/STM_SECTOR_SIZE)	//分给APP的扇区数量
#define APP_MAX_SIZE	0x3E800

iapfun jump2app; 

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}		 


//Flash中用户代码执行
void UserFlashAppRun(void)
{
	printf("开始执行FLASH用户代码!!\r\n");
	if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
	{	 
		iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP1代码
	}else 
	{
		printf("APP程序加载失败!\r\n");   
	}									 
}

uint16_t readBuff[STM_SECTOR_SIZE/2] = {0};
//固件更新函数
void UpdateFun(void)
{
	uint16_t app2FlagBuff[2] = {0xFFFF, 0xFFFF};
	printf("开始更新固件...\r\n");
	for(uint16_t i=0; i<APP_SIZE; i++) {
		printf("正在更新固件%d...\r\n", i);
		STMFLASH_Read(FLASH_APP2_ADDR+i*STM_SECTOR_SIZE, readBuff, STM_SECTOR_SIZE/2);
		STMFLASH_Write(FLASH_APP1_ADDR+i*STM_SECTOR_SIZE, readBuff, STM_SECTOR_SIZE/2);	
		//STMFLASH_WriteEasy(FLASH_APP1_ADDR+i*STM_SECTOR_SIZE, readBuff, STM_SECTOR_SIZE);
	}
	printf("固件更新完成!\r\n");	
	STMFLASH_Write(APP2_FLAG_ADDR, app2FlagBuff, 2);
	UserFlashAppRun();
}

int main(void)
{
	uint16_t app2FlagBuff[2] = {0};
	//设置中断优先级分组：3：1
	NVIC_SetPriorityGrouping(4);
//	SysTick_Init();//系统定时器初始化
	Led_Config();
	Key_Config();
	Beep_Config();
	Usart1_Config();
	printf("Bootloader程序执行\r\n");
	
//1.读APP2区域存放的标志位，如果有0xAAAA,表示APP2中有待更新的程序
	STMFLASH_Read(APP2_FLAG_ADDR, app2FlagBuff, 2);
	if(app2FlagBuff[0] == 0xAAAA && app2FlagBuff[1] == 0xAAAA) {
		//有更新程序
		printf("有更新程序,正在执行代码升级\r\n");
		UpdateFun();   //2.将程序从APP2搬运到APP1
	}
	else {
		//没有新的程序
		printf("没有新的程序,执行原有APP\r\n");
		UserFlashAppRun(); //3.没有新的APP2程序，执行原有的APP1
	}

	while (1)   //如果APP1和APP2都加载失败，执行这部分代码
  {
		LedToggle(Led1Port, Led1Pin);
		LedToggle(Led2Port, Led2Pin);
		LedToggle(Led3Port, Led3Pin);
		LedToggle(Led4Port, Led4Pin);
		Delay_ms(200);
	}
}
