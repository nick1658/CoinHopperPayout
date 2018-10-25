#include "main.h"



int ad8804_init (void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //LED-->PB.9 端口配置
//	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
//	GPIO_SetBits(GPIOB,GPIO_Pin_9);						 //PB.9 输出高

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	    		 //LED1-->PB.8 端口配置, 推挽输出
//	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
//	GPIO_SetBits(GPIOB,GPIO_Pin_8); 						 //PB.8 输出高 
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		 //LED2-->PB.7 端口配置, 推挽输出
//	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
//	GPIO_SetBits(GPIOB,GPIO_Pin_7); 						 //PB.7 输出高 
	
//LED Init
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	    		 //LED1-->PB.8 端口配置, 推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	GPIO_SetBits(GPIOB,GPIO_Pin_0); 						 //PB.8 输出高 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	    		 //LED2-->PB.7 端口配置, 推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	GPIO_SetBits(GPIOB,GPIO_Pin_1); 						 //PB.7 输出高 
	
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 //
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //
	GPIO_SetBits(GPIOA,GPIO_Pin_4); 						 //
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //
	GPIO_SetBits(GPIOA,GPIO_Pin_5); 						 //
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		 //
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //
	GPIO_SetBits(GPIOA,GPIO_Pin_7); 						 //
	return 0;
}

int ad8804_write_ch (uint8_t ch, uint8_t value)
{
	uint8_t mask;
	SPI_SCK = 0;
	SPI_CS = 0;
	for (mask = 0x08; mask > 0; mask >>= 1){
		if (ch & mask){
			SPI_MOSI = 1;
		}else{
			SPI_MOSI = 0;
		}
		SPI_SCK = 1;
		delay_us(SPI_DELAY);
		SPI_SCK = 0;
		delay_us(SPI_DELAY);
	}
	for (mask = 0x80; mask > 0; mask >>= 1){
		if (value & mask){
			SPI_MOSI = 1;
		}else{
			SPI_MOSI = 0;
		}
		SPI_SCK = 1;
		delay_us(SPI_DELAY);
		SPI_SCK = 0;
		delay_us(SPI_DELAY);
	}
	SPI_CS = 1;
	delay_us(SPI_DELAY);
	return 0;
}


