#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "usart.h"
#include "stdio.h"
#include "stmflash.h"

//D:\Keil_v5\Arm\ARMCC\bin\fromelf.exe --bin -o .\Objects\weather.bin .\Objects\weather.axf
//D:\Keil_v5\Arm\ARMCC\bin\fromelf.exe -bin --output  "@L.bin" "#L"

int main(void)
{
	SCB->VTOR = FLASH_BASE | 0x3000;//中断向量表的地址偏移,寄存器写法
//	void NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset);   库函数写法
	
	//设置中断优先级分组：3：1
	NVIC_SetPriorityGrouping(4);
	SysTick_Init();//系统定时器初始化
	Led_Config();
	Key_Config();
	Beep_Config();
	Usart1_Config();
	printf("新的APP正在执行\r\n");
	if(STMFLASH_Erase(FLASH_APP2_ADDR, APP_MAX_SIZE) == 0) {    //执行新的代码之后，清楚APP2的空间，已备下一次升级
		printf("APP备份区域擦除成功\r\n");
	}
	while (1)
  {
		RecvOverFun();   //处理IAP下发的升级文件
		if(KeyRunTime[0] > KeyRunTime[1]) {
			switch(Get_KeyValue()) {
				case 1:
					printf("按键1\r\n");
					BeepOn();
					break;
				case 2:
					printf("按键2\r\n");
					BeepOff();
					break;
			}
			KeyRunTime[0] = 0;
		}
		if(Led1RunTime[0] > Led1RunTime[1]) {//500ms
			LedToggle(Led1Port, Led1Pin);
			printf("APP执行\r\n");
			Led1RunTime[0] = 0;
		}	
	}
}
