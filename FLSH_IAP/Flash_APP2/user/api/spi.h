#ifndef __SPI_H
#define __SPI_H


#include "stm32f10x.h"

#define sFLASH_SPI                       SPI2
#define sFLASH_SPI_CLK                   RCC_APB1Periph_SPI2
#define sFLASH_SPI_APB_CMD							 RCC_APB1PeriphClockCmd


#define sFLASH_SPI_SCK_PIN               GPIO_Pin_13                 
#define sFLASH_SPI_SCK_GPIO_PORT         GPIOB                       
#define sFLASH_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOB

#define sFLASH_SPI_MISO_PIN              GPIO_Pin_14                 
#define sFLASH_SPI_MISO_GPIO_PORT        GPIOB                      
#define sFLASH_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOB

#define sFLASH_SPI_MOSI_PIN              GPIO_Pin_15               
#define sFLASH_SPI_MOSI_GPIO_PORT        GPIOB                      
#define sFLASH_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOB

#define sFLASH_CS_PIN                    GPIO_Pin_12                 
#define sFLASH_CS_GPIO_PORT              GPIOB                   
#define sFLASH_CS_GPIO_CLK               RCC_APB2Periph_GPIOB


void Spi2_Config(void);



#endif

