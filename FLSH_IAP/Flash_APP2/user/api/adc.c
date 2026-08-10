#include "adc.h"


uint16_t AdcArr[20] = {0};


//光敏电阻	ADC1 IN5		
//					PA5
//MQ2				ADC1 IN11
//					PC1
void Adc_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_DeInit(DMA1_Channel1);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//设置外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AdcArr[0];//设置存储器基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//设置传输方向  外设--存储器
  DMA_InitStructure.DMA_BufferSize = sizeof(AdcArr)/sizeof(AdcArr[0]);//设置DMA的数据传输总数
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不增加
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器地址递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//循环模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMA通道优先级
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  DMA_Cmd(DMA1_Channel1, ENABLE);//使能DMA
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//配置ADC1   ADCCLK配置
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	ADC_InitTypeDef ADC_InitStructure;
 /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//开扫描
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//循环转换
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//没有外部触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据对齐 右对齐
  ADC_InitStructure.ADC_NbrOfChannel = 2;//规则通道的通道的数量
  ADC_Init(ADC1, &ADC_InitStructure);

  //配置规则通道  通道5
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);
	
  //打开DMA
  ADC_DMACmd(ADC1, ENABLE);
  
  //使能ADC
  ADC_Cmd(ADC1, ENABLE);

  /* 使能ADC1的复位校准 */   
  ADC_ResetCalibration(ADC1);
  /* 等待校准完成 */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* 使能ADC1校准 */
  ADC_StartCalibration(ADC1);
  /* 等待校准完成 */
  while(ADC_GetCalibrationStatus(ADC1));
	
	//启动转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}



void Get_AdcValue(__ADC_VALUE_TypeDef * adcValue)
{
	float avgLight = 0;
	float avgMQ2 = 0;
	for(uint8_t i=0; i<20; i+=2) {
		avgLight += AdcArr[i]/10.0;
		avgMQ2 += AdcArr[i+1]/10.0;
	}
	adcValue->Light = avgLight;
	adcValue->MQ2 = avgMQ2;
}




