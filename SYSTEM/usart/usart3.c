#include "main.h"


void uart3_send_data (int ch)
{
	//USART3->SR;  //USART_GetFlagStatus(USART3, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(USART3, '\r');
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	}
	//һ��һ�������ַ�
	USART_SendData(USART3, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
}
void uart3_send_byte (uint8_t ch)
{
	//USART3->SR;  //USART_GetFlagStatus(USART3, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	//һ��һ�������ַ�
	USART_SendData(USART3, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
}

void uart3_puts(const char *s)
{
	while (*s) {
		uart3_send_data(*s++);
	}
}

void uart3_dma_config (void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//������DMA����    
    //����DMAʱ��  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
    //DMA1ͨ��5����  
    DMA_DeInit(DMA1_Channel3);  
    //�����ַ  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);  
    //�ڴ��ַ  
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)cmd_analyze.rec_buf3;  
    //dma���䷽����  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
    //����DMA�ڴ���ʱ�������ĳ���  
    DMA_InitStructure.DMA_BufferSize = CMD_BUF_LEN;  
    //����DMA���������ģʽ��һ������  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
    //����DMA���ڴ����ģʽ  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
    //���������ֳ�  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
    //�ڴ������ֳ�  
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
    //����DMA�Ĵ���ģʽ  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
    //����DMA�����ȼ���  
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
    //����DMA��2��memory�еı����������  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
    DMA_Init(DMA1_Channel3,&DMA_InitStructure);  
		
		 
	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE); //��������ж�

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_5_INT_PREEM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_5_INT_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
		
    //ʹ��ͨ��3  
    DMA_Cmd(DMA1_Channel3,ENABLE);  
    //����DMA��ʽ����  
    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);  
}
//����1 DMA��ʽ�����ж�  
void DMA1_Channel2_IRQHandler(void)  
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif	
	if(DMA_GetITStatus(DMA1_IT_TC2)){
		//�����־λ  
		DMA_ClearFlag(DMA1_FLAG_TC2);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA1_IT_GL2); //���ȫ���жϱ�־ 
		USART_DMACmd(USART3,USART_DMAReq_Tx, DISABLE); //DISABLE����1��DMA����  
		DMA_Cmd(DMA1_Channel2, DISABLE);  
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
//����1 DMA��ʽ�������ж�
void DMA1_Channel3_IRQHandler(void)  
{  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(DMA_GetITStatus(DMA1_IT_TC3)){
		//�����־λ  
		DMA_ClearFlag(DMA1_FLAG_TC3);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA1_IT_GL3); //���ȫ���жϱ�־ 
		my_env.uart_receive_finished3 = 2;
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}
//

void start_uart3_dma (u32 _memBaseAddr, U16 _size)
{
	/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
	/* Write to DMA1 Channel4 CMAR */
	DMA1_Channel2->CMAR = _memBaseAddr;
	DMA_SetCurrDataCounter (DMA1_Channel2, _size);
	DMA_Cmd(DMA1_Channel2, ENABLE);  
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���1��DMA����   
}

void uart3_init (void)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3|RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART3��GPIOAʱ��
  
	//USART3_TX   GPIOB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10
   
	//USART3_RX	  GPIOB.11��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.11  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART3_RX_INT_PREEM;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART3_RX_INT_SUB;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = UART3_BOUND_RATE;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж� 
	USART_ITConfig(USART3,USART_IT_IDLE,ENABLE); //�������ڿ����ж� 
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3
	
	uart3_dma_config ();

}


void stop_uart3_receive (void)
{
	DMA_Cmd(DMA1_Channel3,DISABLE);
	USART_ITConfig(USART3,USART_IT_IDLE, DISABLE); //�������ڿ����ж� 
}
void start_uart3_receive (void)
{        
	//���ô������ݳ���  
	memset (cmd_analyze.rec_buf3, 0, CMD_BUF_LEN);
	DMA_Cmd(DMA1_Channel3,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel3, CMD_BUF_LEN);  
	DMA_Cmd(DMA1_Channel3,ENABLE);
	USART_ITConfig(USART3,USART_IT_IDLE, ENABLE); //�������ڿ����ж� 
}


#if EN_USART3_RX   //���ʹ���˽���
//����1�����ж�     
void USART3_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET){  
		//USART_ClearFlag(USART3,USART_IT_IDLE);  
		temp = USART3->SR;  
		temp = USART3->DR; //��USART_IT_IDLE��־  
		temp = temp;
		my_env.uart_receive_finished3 = 1;
	}   
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif 
} 
#endif
