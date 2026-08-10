#include "stm32f10x.h"
#include "delay.h"
#include "app.h"
#include "string.h"


uint8_t Device_EnterSetMode(void)
{
	uint16_t cnt = 5000;
	printf("ÇëÔÚ5sÄÚ°´ÏÂ°´¼ü4£¬½øÈëÉèÖÃÄ£Ê½\r\n");
	while(cnt--) {
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == Bit_RESET)
		{
			vTaskDelay(10);
			if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == Bit_RESET) {
				return 1;
			}
		}
		vTaskDelay(1);
	}
	return 0;
}

void Device_SetMode(void)
{
	uint8_t sta = 0;
	//½øĞĞÉèÖÃÄ£Ê½µÄÅäÖÃ
	printf("Enter Setting Mode\r\n");
	Esp8266_SendATCMD("AT+CWMODE=2\r\n", "OK", 500);//ÉèÖÃÎªAPÄ£Ê½
	//ÉèÖÃÈÈµãµÄÃû×ÖºÍÃÜÂë
	Esp8266_SendATCMD("AT+CWSAP_DEF=\"XYD_WifiWeather\",\"12345678\",5,3\r\n", "OK", 500);
	//¸´Î»
	Esp8266_SendATCMD("AT+RST\r\n", "ready", 10000);
	//ÉèÖÃ¶àÁ¬½Ó
	Esp8266_SendATCMD("AT+CIPMUX=1\r\n", "OK", 10000);
	//ÉèÖÃ·şÎñÆ÷µÄ¶Ë¿Ú--Ä¬ÈÏµÄIP  192.168.2.50
	Esp8266_SendATCMD("AT+CIPSERVER=1,8080\r\n", "OK", 10000);//ÉèÖÃ¶Ë¿Ú
	printf("Çë´ò¿ªAPP£¬ÉèÖÃĞÅÏ¢\r\n");
	memset(espData.recvBuff, 0, ESP_RECV_MAX_SIZE);
	espData.recvCnt = 0;
	while(1)
	{
		switch(sta)
		{
			case 0:
				printf("ÇëÏÈÁ¬½ÓÉè±¸ÊÍ·ÅµÄÈÈµã\r\n");
				if(strstr((char*)espData.recvBuff, "+STA_CONNECTED") != NULL) {
					memset(espData.recvBuff, 0, ESP_RECV_MAX_SIZE);
					espData.recvCnt = 0;
					sta++;
				}
				break;
			case 1:
				printf("ÇëÊ¹ÓÃAPP,Á¬½Ó·şÎñÆ÷\r\n");
				if(strstr((char*)espData.recvBuff, "CONNECT") != NULL) {
					memset(espData.recvBuff, 0, ESP_RECV_MAX_SIZE);
					espData.recvCnt = 0;
					sta++;
				}
				break;
			case 2:
				printf("ÇëÊ¹ÓÃAPP·¢ËÍÊı¾İ\r\n");
				if(strstr((char*)espData.recvBuff, "+IPD") != NULL) {
					printf("%s\r\n", strstr((char*)espData.recvBuff, "{"));
					return;
				}
				break;
		}
		vTaskDelay(500);
	}
}


lv_obj_t * scr1 = NULL;
lv_obj_t * scr2 = NULL;


static void Scr1EventCb(struct _lv_obj_t * obj, lv_event_t event)
{
	switch(event) {
			case LV_EVENT_RELEASED://æŒ‰ä¸‹å¹¶æ¾å¼€äº‹ä»¶
				printf("key%d Released\r\n", (uint8_t)lv_event_get_data());
				break;
	}
}



void Bsp_Init(void)
{

	//ÉèÖÃÖĞ¶ÏÓÅÏÈ¼¶·Ö×é£º3£º1
	NVIC_SetPriorityGrouping(4);
	Led_Config();
	Key_Config();
	Beep_Config();
	Usart1_Config();
	printf("usart is ok\r\n");
	
	printf("ĞÂµÄAPPÕıÔÚÖ´ĞĞ\r\n");
	if(STMFLASH_Erase(FLASH_APP2_ADDR, APP_MAX_SIZE) == 0) {
		printf("APP±¸·İÇøÓò²Á³ı³É¹¦\r\n");
	}
	
	
	Adc_Config();
	ESP8266_Init();
	
	Usart5_Config();

	
	lv_init();//LVGLµÄ³õÊ¼»¯
	lv_port_disp_init();//lvgl³õÊ¼»¯ÏÔÊ¾Éè±¸

	scr1 = lv_obj_create(NULL, NULL);//åˆ›å»ºä¸€ä¸ªå±å¹•
	
	lv_obj_set_size(scr1, 320, 240);//è®¾ç½®æŸä¸ªå¯¹è±¡çš„å°ºå¯¸
	lv_obj_set_pos(scr1, 0, 0);  //è®¾ç½®å¯¹è±¡çš„ä½ç½® å·¦ä¸Šè§’çš„åƒç´ ä½ç½®
		
	lv_obj_t * label1 = lv_label_create(scr1, NULL);//åœ¨å½“å‰å±å¹•ä¸Šåˆ›å»ºä¸€ä¸ªæ–‡æœ¬
	lv_label_set_long_mode(label1, LV_LABEL_LONG_SROLL_CIRC);     
	lv_label_set_recolor(label1, true); //æ–‡å­—é‡æ–°ç€è‰²                     
	lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label "
																										"and  wrap long text automatically." LV_SYMBOL_OK LV_SYMBOL_WIFI LV_SYMBOL_PLAY);
	lv_obj_set_width(label1, 150);
	lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);




	scr2 = lv_obj_create(NULL, NULL);//åˆ›å»ºä¸€ä¸ªå±å¹•
	
	lv_obj_set_size(scr2, 320, 240);//è®¾ç½®æŸä¸ªå¯¹è±¡çš„å°ºå¯¸
	lv_obj_set_pos(scr2, 0, 0);  //è®¾ç½®å¯¹è±¡çš„ä½ç½® å·¦ä¸Šè§’çš„åƒç´ ä½ç½®
		
	lv_obj_t * label2 = lv_label_create(scr2, NULL);//åœ¨å½“å‰å±å¹•ä¸Šåˆ›å»ºä¸€ä¸ªæ–‡æœ¬
	lv_label_set_long_mode(label2, LV_LABEL_LONG_SROLL_CIRC);                         
	lv_label_set_align(label2, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(label2, "HWG");
	lv_obj_set_width(label2, 150);
	lv_obj_align(label2, NULL, LV_ALIGN_CENTER, 0, -30);



//	lv_scr_load(scr1);//åŠ è½½å±å¹•
	//ä»¥åŠ¨ç”»çš„å½¢å¼åŠ è½½å±å¹•
	lv_scr_load_anim(scr1, LV_SCR_LOAD_ANIM_OVER_LEFT, 2000, 0, true);	
	
	sFLASH_Init();
	printf("W25Q ID:%X", sFLASH_ReadID());
	
//	if(Device_EnterSetMode() == 1) {
//		Device_SetMode();
//	}
	Rtc_Config();
}

int main(void)
{
	SCB->VTOR = FLASH_BASE | 0x3000;//ÖĞ¶ÏÏòÁ¿±íµÄµØÖ·Æ«ÒÆ
	BaseType_t xReturnedStart = xTaskCreate( vTaskStartCode,"start task",256,NULL,0,&xStartTaskHandle);
	if(xReturnedStart == pdPASS) {
		vTaskStartScheduler();
	}
	while (1)
  {}
}


