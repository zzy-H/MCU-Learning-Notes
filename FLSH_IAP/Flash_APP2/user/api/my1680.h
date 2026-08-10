#ifndef __MY1680_H
#define __MY1680_H

#include "stm32f10x.h"

typedef struct {
	uint8_t head;
	uint8_t len;
	uint8_t cmd;
	uint8_t param[2];
	uint8_t paramLen;
	uint8_t check;
	uint8_t end;
}__MY1680_DATA_TypeDef;

#define MY1680_PLAY_DIRFILE_CMD	 		0x42
#define MY1680_PLAY_ROOTFILE_CMD	 	0x41


void Usart5_Config(void);
void Usart5_SendBuff(uint8_t * buff, uint32_t len);


void My1680_Init(void);
void My1680_SendCMDData(__MY1680_DATA_TypeDef myData);
void My1680_SetVoiceSize(uint8_t size);
void My1680_PlayDirFile(uint8_t dirNum, uint8_t fileNum);
void My1680_PlayNum(uint8_t num);
#endif
