#include "rtc.h"
#include "stdio.h"

void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_SEC);
    RTC_WaitForLastTask();
  }	
}

void Rtc_Config(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//配置RTC中断的NVIC
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	//读取备份区域DR1内的数据 不为0xA5A5就表示RTC没有配置过，就进入配置模式
  if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
		//开电源控制器以及备份区的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		//使能备份区域访问
		PWR_BackupAccessCmd(ENABLE);
		BKP_DeInit();//复位备份区域
		
		RCC_LSEConfig(RCC_LSE_ON);//打开外部低速时钟
		//等待LSE时钟稳定
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{}
		//选择LSE作为RTC的时钟源
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		//RTC时钟使能
		RCC_RTCCLKCmd(ENABLE);
		//等待时钟同步
		RTC_WaitForSynchro();
		//等待对RTC的写操作完成
		RTC_WaitForLastTask();

		//使能秒中断
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		//等待对RTC的写操作完成
		RTC_WaitForLastTask();
		//设置RTC的分频系数
		RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
		//等待对RTC的写操作完成
		RTC_WaitForLastTask();
		
    printf("\r\n RTC configured....");

		//等待对RTC的写操作完成
		RTC_WaitForLastTask();
		//设置RTC的当前计数值---设置了当前时间
		RTC_SetCounter(0);
		//等待对RTC的写操作完成
		RTC_WaitForLastTask();

    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  }
  else
  {
    printf("\r\n No need to configure RTC....");
    //等待时钟同步完成
    RTC_WaitForSynchro();
    //使能秒中断
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    //等待操作完成
    RTC_WaitForLastTask();
  }
}


//设置从1970年后到现在的s数
void Rtc_SetTimeSec(uint32_t sec)
{
	//开电源控制器以及备份区的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//使能备份区域访问
	PWR_BackupAccessCmd(ENABLE);
	//等待时钟同步
	RTC_WaitForSynchro();
	//等待对RTC的写操作完成
	RTC_WaitForLastTask();
	//设置RTC的当前计数值---设置了当前时间
	RTC_SetCounter(sec);
	//等待对RTC的写操作完成
	RTC_WaitForLastTask();
	//关闭备份区域访问
	PWR_BackupAccessCmd(DISABLE);
}








