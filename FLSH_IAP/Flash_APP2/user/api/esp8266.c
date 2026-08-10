#include "esp8266.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "cJSON.h"
#include "FreeRTOS.h"
#include "task.h"


__ESP8266_DATATypedef espData = {0};
void USART3_IRQHandler(void)
{
	uint8_t data = 0;
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
		data = USART3->DR;//读取RX引脚收到的数据
		USART1->DR = data;
		if(data == '\0') data = '0';
		espData.recvBuff[espData.recvCnt++] = data;
		espData.recvCnt %= ESP_RECV_MAX_SIZE;
	}
	if(USART_GetFlagStatus(USART3, USART_FLAG_IDLE) == SET) {
		data = USART3->SR;
		data = USART3->DR;
	}
}


//ESP8266 -- STM32 USART3
//USART3  PB10	PB11
//ENABLE	PE6

//底层通信接口


//USART3	TX PB10	RX PB11
void Usart3_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不开硬件控制流
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//发送器和接收器使能
	USART_InitStructure.USART_Parity = USART_Parity_No;//没有奇偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8个数据位
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//打开USART3的RXNE中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//打开空闲中断
	NVIC_SetPriority(USART3_IRQn, 1);
	NVIC_EnableIRQ(USART3_IRQn);
	
	USART_Cmd(USART3, ENABLE);//使能USART3
}


void Usart3_SendByte(uint8_t data)
{
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) 
	{}
	USART_SendData(USART3, data);	
}

void Usart3_SendBuff(uint8_t * buff, uint32_t len)
{
	for(uint32_t i=0; i<len; i++) {
		Usart3_SendByte(buff[i]);
	}
}


void ESP8266_Init(void)
{
	Usart3_Config();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	ESP8266_ENABLE();
//	Delay_ms(10000);
}


void Esp8266_SendString(char *str)
{
	while(*str != '\0') {
		Usart3_SendByte(*str++);
	}
}


//str1  比较大的字符串
//str2  比较短的字符串   
//	hello world       wo
char * FindStr(uint8_t *str1, char *str2, uint32_t timeOut)
{
	uint32_t time = timeOut;
	while(strstr((char*)str1, str2) == NULL)
	{
		time--;
		if(time == 0) break;
		vTaskDelay(1);
	}
	return strstr((char*)str1, str2);
}



//esp8266 发送指令，并查询是否成功
//1 成功了  0 失败了
uint8_t Esp8266_SendATCMD(char * cmd, char * recv, uint32_t timeOut)
{
	uint8_t renValue = 0;
	memset(espData.recvBuff, 0, ESP_RECV_MAX_SIZE);
	espData.recvCnt = 0;
	Esp8266_SendString(cmd);
	
	if(recv == NULL)	{
		renValue = 1;
		vTaskDelay(timeOut);
		return renValue;
	}
	
	if(FindStr(espData.recvBuff, recv, timeOut) != NULL) {
		renValue = 1;
	}
	return renValue;
}

//解析当前天气数据
void cJson_AnalysisNowData(void)
{
	//根据接收数组，解析数据，会生成一个json的链表头
	cJSON* cjsonRoot = cJSON_Parse((char *)espData.recvBuff);
	if(cjsonRoot == NULL) {//解析失败
		printf("cjsonRoot Err\r\n");
	}
	else {//解析成功
		printf("cjsonRoot Success\r\n");
		//根据json链表头，查找第一个键值对：results
		cJSON* cjsonResults = cJSON_GetObjectItem(cjsonRoot, "results");
		if(cjsonResults == NULL) printf("cjsonResults Err\r\n");
		else {
			printf("cjsonResults Success,Arr size:%d\r\n", cJSON_GetArraySize(cjsonResults));
			
			//获取results数组的第0个元素
			cJSON* cjsonResults_0 = cJSON_GetArrayItem(cjsonResults, 0);
			if(cjsonResults_0 == NULL) printf("cjsonResults_0 Err\r\n");
			else {
				//从results数组的第0个元素中获取now这个json对象
				cJSON* cjsonNow = cJSON_GetObjectItem(cjsonResults_0, "now");		
				//提取now对象中的数据
				printf("text:%s\r\n", cJSON_GetObjectItem(cjsonNow, "text")->valuestring);
				printf("code:%s\r\n", cJSON_GetObjectItem(cjsonNow, "code")->valuestring);
				printf("tem:%s\r\n", cJSON_GetObjectItem(cjsonNow, "temperature")->valuestring);
			}
		}
	}
	cJSON_Delete(cjsonRoot);//必须加入！！！！！
}




void Weather_GetDate(void)
{
	static uint8_t cnt = 0;
	switch(cnt) {
		case 0:
			if(Esp8266_SendATCMD("AT\r\n", "OK", 1000) == 1) {
				cnt++;
			}
		break;
		case 1:
			if(Esp8266_SendATCMD("AT+RST\r\n", "ready", 10000) == 1) {
				cnt++;
			}
		break;	
		case 2:
			if(Esp8266_SendATCMD("AT+CWMODE=1\r\n", "OK", 10000) == 1) {
				cnt++;
			}
		break;
		case 3:
			if(Esp8266_SendATCMD("AT+CWJAP=\"xyd\",\"12345678\"\r\n", "OK", 10000) == 1) {
				cnt++;
			}		
		break;
		case 4:
			if(Esp8266_SendATCMD("AT+CIPSTART=\"TCP\",\"116.62.81.138\",80\r\n", "OK", 10000) == 1) {
				cnt++;
			}		
		break;
		case 5:
			if(Esp8266_SendATCMD("AT+CIPMODE=1\r\n", "OK", 10000) == 1) {
				cnt++;
			}	
		break;
		case 6:
			if(Esp8266_SendATCMD("AT+CIPSEND\r\n", ">", 10000) == 1) {
				cnt++;
			}
		break;
		case 7:
			memset(espData.recvBuff, 0, ESP_RECV_MAX_SIZE);
			espData.recvCnt = 0;
			Esp8266_SendString("GET https://api.seniverse.com/v3/weather/now.json?key=Sr35f1GmIqaGkjD_t&location=zhengzhou&language=en&unit=c\r\n");
			cnt++;
		break;
		case 8:
			cJson_AnalysisNowData();
			if(Esp8266_SendATCMD("+++", NULL, 1000) == 1) {
				cnt=0;
			}	
		break;	
	}
	
}




