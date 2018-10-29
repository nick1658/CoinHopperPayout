#include "main.h"



int led_init (void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	
//LED Init
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	    		 //LED1-->PB.8 �˿�����, �������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOB,GPIO_Pin_0); 						 //PB.8 ����� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	    		 //LED2-->PB.7 �˿�����, �������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOB,GPIO_Pin_1); 						 //PB.7 ����� 
	
	return 0;
}

