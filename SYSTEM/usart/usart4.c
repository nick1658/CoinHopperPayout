#include "main.h"


void uart4_send_data (int ch)
{
	//UART4->SR;  //USART_GetFlagStatus(UART4, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(UART4,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(UART4, '\r');
		while(USART_GetFlagStatus(UART4,USART_FLAG_TC)!=SET);
	}
	//һ��һ�������ַ�
	USART_SendData(UART4, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(UART4,USART_FLAG_TC)!=SET);
}
void uart4_send_byte (uint8_t ch)
{
	//UART4->SR;  //USART_GetFlagStatus(UART4, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(UART4,USART_FLAG_TC)!=SET);
	//һ��һ�������ַ�
	USART_SendData(UART4, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(UART4,USART_FLAG_TC)!=SET);
}

void uart4_puts(const char *s)
{
	while (*s) {
		uart4_send_data(*s++);
	}
}

void uart4_dma_config (void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//������DMA����    
    //����DMAʱ��  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);  
    //DMA1ͨ��5����  
    DMA_DeInit(DMA2_Channel3);  
    //�����ַ  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);  
    //�ڴ��ַ  
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)cmd_analyze.rec_buf4;  
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
    DMA_Init(DMA2_Channel3,&DMA_InitStructure);  
		
		 
	DMA_ITConfig(DMA2_Channel3, DMA_IT_TC, ENABLE); //��������ж�

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_5_INT_PREEM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_5_INT_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
		
    //ʹ��ͨ��5  
    DMA_Cmd(DMA2_Channel3,ENABLE);  
    //����DMA��ʽ����  
    USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE);  
}
//����1 DMA��ʽ�����ж�  
void DMA2_Channel5_IRQHandler(void)  
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif	
	if(DMA_GetITStatus(DMA2_IT_TC5)){
		//�����־λ  
		DMA_ClearFlag(DMA2_FLAG_TC5);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA2_IT_GL5); //���ȫ���жϱ�־ 
		USART_DMACmd(UART4,USART_DMAReq_Tx, DISABLE); //DISABLE����1��DMA����  
		DMA_Cmd(DMA2_Channel5, DISABLE);  
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
//����1 DMA��ʽ�������ж�
void DMA2_Channel3_IRQHandler(void)  
{  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(DMA_GetITStatus(DMA2_IT_TC3)){
		//�����־λ  
		DMA_ClearFlag(DMA2_FLAG_TC3);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA2_IT_GL3); //���ȫ���жϱ�־ 
		my_env.uart_receive_finished4 = 2;
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}
//

void start_uart4_dma (u32 _memBaseAddr, U16 _size)
{
	/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
	/* Write to DMA1 Channel4 CMAR */
	DMA2_Channel5->CMAR = _memBaseAddr;
	DMA_SetCurrDataCounter (DMA2_Channel5, _size);
	DMA_Cmd(DMA2_Channel5, ENABLE);  
	USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���1��DMA����   
}

void uart4_init (void)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//ʹ��UART4ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
  
	//UART4_TX   GPIOC.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.10
   
	//UART4_RX	  GPIOC.11��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.11
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,DISABLE);

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART4_RX_INT_PREEM;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART4_RX_INT_SUB;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = UART4_BOUND_RATE;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(UART4, &USART_InitStructure); //��ʼ������4
	//USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�������ڽ����ж� 
	USART_ITConfig(UART4,USART_IT_IDLE,ENABLE); //�������ڿ����ж� 
	USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���4 
	
	uart4_dma_config ();

}


void stop_uart4_receive (void)
{
	DMA_Cmd(DMA2_Channel3,DISABLE);
	USART_ITConfig(UART4,USART_IT_IDLE, DISABLE); //�������ڿ����ж� 
}
void start_uart4_receive (void)
{        
	//���ô������ݳ���  
	memset (cmd_analyze.rec_buf4, 0, CMD_BUF_LEN);
	DMA_Cmd(DMA2_Channel3,DISABLE);
	DMA_SetCurrDataCounter(DMA2_Channel3, CMD_BUF_LEN);  
	DMA_Cmd(DMA2_Channel3,ENABLE);
	USART_ITConfig(UART4,USART_IT_IDLE, ENABLE); //�������ڿ����ж� 
}


#if EN_UART4_RX   //���ʹ���˽���
//����1�����ж�     
void UART4_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET){  
		//USART_ClearFlag(UART4,USART_IT_IDLE);  
		temp = UART4->SR;  
		temp = UART4->DR; //��USART_IT_IDLE��־  
		temp = temp;
		my_env.uart_receive_finished4 = 1;
	}   
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif 
} 
#endif
