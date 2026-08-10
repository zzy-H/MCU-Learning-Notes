#include "rgb.h"

//RGB -- 3个
//	R			G				B
//PA8	 		PA7			PA6
//TIM1_1	TIM3_2	TIM3_1
//低电平亮
//利用定时器的输出比较 输出PWM 控制灯的亮度，然后组合颜色
void RGB_Config(void)
{
//	B
//	PA6	
//	TIM3_1	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//1 开时钟  TIM3、TIM1	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	//2 配置时钟源72MHz、分频器、装载值
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 1000-1;
  TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	// 重装载值预装载使能
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	
	TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//设置输出模式
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//使能输出
  TIM_OCInitStructure.TIM_Pulse = 0;//比较值
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//有效电平  低
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);//配置通道1
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);//配置通道2
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);

	//使能定时器
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
}


void RGB_SetColor(uint16_t rPulse, uint16_t gPulse, uint16_t bPulse)
{
	TIM_SetCompare1(TIM1, rPulse);
	TIM_SetCompare1(TIM3, bPulse);
	TIM_SetCompare2(TIM3, gPulse);
}






