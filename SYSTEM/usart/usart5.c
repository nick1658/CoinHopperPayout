#include "main.h"


void uart5_send_data (int ch)
{
	//UART5->SR;  //USART_GetFlagStatus(UART5, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(UART5, '\r');
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	}
	//һ��һ�������ַ�
	USART_SendData(UART5, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
}
void uart5_send_byte (uint8_t ch)
{
	//UART5->SR;  //USART_GetFlagStatus(UART5, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	//һ��һ�������ַ�
	USART_SendData(UART5, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
}

void uart5_puts(const char *s)
{
	while (*s) {
		uart5_send_data(*s++);
	}
}


void uart5_init (void)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ��UART5ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PC�˿�ʱ��
  
	//UART5_TX   GPIOC.12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.12
   
	//UART5_RX	  GPIOD.2��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PC.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOC.11
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5,DISABLE);

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART5_RX_INT_PREEM;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART5_RX_INT_SUB;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = UART5_BOUND_RATE;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(UART5, &USART_InitStructure); //��ʼ������5
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�������ڽ����ж� 
	//USART_ITConfig(UART5,USART_IT_IDLE,ENABLE); //�������ڿ����ж� 
	USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ���5 

}



#if EN_UART5_RX   //���ʹ���˽���
//����1�����ж�     
void UART5_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET){  
		//USART_ClearFlag(UART5,USART_IT_IDLE);  
		temp = UART5->SR;  
		temp = UART5->DR; //��USART_IT_RXNE��־  
		cmd_analyze.uart5_ctr %=CMD_BUF_LEN;
		cmd_analyze.rec_buf5[cmd_analyze.uart5_ctr++] = temp;
		cmd_analyze.uart5_recv_time_out = UART5_RECV_TIMEOUT;
	}   
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif 
} 
#endif
