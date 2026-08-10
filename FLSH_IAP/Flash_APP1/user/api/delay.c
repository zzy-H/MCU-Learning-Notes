#include "delay.h"

vu32 SystemTime = 0;
vu32 Led1RunTime[2] = {0, 500};
vu32 KeyRunTime[2] = {0, 10};
vu32 Led2RunTime[2] = {0, 700};

void SysTick_Handler(void)
{
	RecvTimeOut();
	SystemTime++;
	Led1RunTime[0]++;
	KeyRunTime[0]++;
	Led2RunTime[0]++;
}


void Delay_nms(uint32_t t)
{
	uint32_t time = SystemTime + t;
	while(time > SystemTime)
	{}
}

//利用系统滴答定时器构造一个1ms的定时
void SysTick_Init(void)
{
	SysTick_Config(72000);
}

vu32 usTick = 0;
void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET) {
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		usTick--;
	}
}

//定时器构造的us级延时
void Delay_nus(uint32_t t)
{
	usTick = t;
	TIM_SetCounter(TIM6, 1);
	TIM_Cmd(TIM6, ENABLE);
	while(usTick > 0);
}

void Tim6_Config(void)
{
	//1s一次的中断
	//1 开时钟  TIM6		APB1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	//2 配置时钟源72MHz、分频器、装载值
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 36-1;
  TIM_TimeBaseStructure.TIM_Prescaler = 2-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	//3 配置TIM6的中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	//4 配置NVIC
	NVIC_SetPriority(TIM6_IRQn, 0);
	NVIC_EnableIRQ(TIM6_IRQn);
	//使能定时器
	TIM_Cmd(TIM6, DISABLE);
}

void Delay_us(uint32_t t)
{
	while(t--) {
		delay_1us();
	}
}

void Delay_ms(uint32_t t)
{
	uint64_t T = t*1000;
	while(T--) {
		delay_1us();
	}
}












