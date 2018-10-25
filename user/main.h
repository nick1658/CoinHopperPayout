#ifndef MAIN_H
#define MAIN_H

#define U32 unsigned int
#define U16 unsigned short
#define S32 int
#define S16 short int
#define U8  unsigned char
#define	S8  char

#include "stdio.h"	
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stm32f10x.h>
//#include "gpio.h"
#include "timer.h"
#include "stm32f10x.h"
#include "my_cmd.h"

#include "ad8804.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"




#define ARG_NUM     8          //命令中允许的参数个数  
#define CMD_LEN     20         //命令名占用的最大字符长度  
#define CMD_BUF_LEN 60         //命令缓存的最大长度  

//GROUP 0***************************
#define INT_GROUP_0 0
#define TIM5_INT_PREEM INT_GROUP_0
#define TIM5_INT_SUB 0
#define ADC1_2_INT_PREEM INT_GROUP_0
#define ADC1_2_INT_SUB 1

//GROUP 1***************************
#define INT_GROUP_1 1
#define DMA1_1_INT_PREEM INT_GROUP_1
#define DMA1_1_INT_SUB 2
#define TIM2_INT_PREEM INT_GROUP_1
#define TIM2_INT_SUB 3

//GROUP 2***************************
#define INT_GROUP_2 2
#define SDIO_INT_PREEM INT_GROUP_2
#define SDIO_INT_SUB 1
#define DMA1_2_INT_PREEM INT_GROUP_2
#define DMA1_2_INT_SUB 1
#define DMA1_4_INT_PREEM INT_GROUP_2
#define DMA1_4_INT_SUB 1
#define EXT0_INT_PREEM INT_GROUP_2
#define EXT0_INT_SUB 1
#define EXT1_INT_PREEM INT_GROUP_2
#define EXT1_INT_SUB 1
#define EXT2_INT_PREEM INT_GROUP_2
#define EXT2_INT_SUB 1
#define EXT3_INT_PREEM INT_GROUP_2
#define EXT3_INT_SUB 1
#define EXT6_INT_PREEM INT_GROUP_2
#define EXT6_INT_SUB 1
#define EXT7_INT_PREEM INT_GROUP_2
#define EXT7_INT_SUB 1
#define EXT8_INT_PREEM INT_GROUP_2
#define EXT8_INT_SUB 1
#define EXT11_INT_PREEM INT_GROUP_2
#define EXT11_INT_SUB 1
#define UART3_RX_INT_PREEM INT_GROUP_2
#define UART3_RX_INT_SUB 2
#define UART1_RX_INT_PREEM INT_GROUP_2
#define UART1_RX_INT_SUB 3
#define DMA1_5_INT_PREEM INT_GROUP_2
#define DMA1_5_INT_SUB 3

//GROUP 3***************************
#define INT_GROUP_3 3
#define RTC_INT_PREEM INT_GROUP_3
#define RTC_INT_SUB 0
#define TIM3_INT_PREEM INT_GROUP_3
#define TIM3_INT_SUB 2
#define TIM4_INT_PREEM INT_GROUP_3
#define TIM4_INT_SUB 3
#define TIM7_INT_PREEM INT_GROUP_3
#define TIM7_INT_SUB 3
#define DMA2_3_INT_PREEM INT_GROUP_3
#define DMA2_3_INT_SUB 3



#define LED0 PBout(0)// PB0
#define LED1 PBout(1)// PB1
#define LED2 PBout(9)// PB9

#define SHDL PBout(6)
#define SPI_CS PAout(4)
#define SPI_SCK PAout(5)
#define SPI_MISO PAout(6)
#define SPI_MOSI PAout(7)

#define MY_PRINT	
#ifdef MY_PRINT
#define my_print(format, arg...) printf("" format "", ## arg)
#define my_println(format, arg...) printf("" format "\n", ## arg)
#define cmd(format, arg...) printf("Nick-Cmd:" format "", ## arg)
#else
#define my_println(format, arg...) 
#define cmd(format, arg...) 
#endif


#define CHANEL_NUM 12 //为12个通道

typedef struct {
	uint8_t da_addr[CHANEL_NUM];
	uint8_t da_value[CHANEL_NUM];
}s_ad8804_env;


extern s_ad8804_env ad8804_env;


#endif

