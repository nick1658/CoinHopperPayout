#include "main.h"


void uart1_send_data (int ch)
{
	//USART1->SR;  //USART_GetFlagStatus(USART1, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(USART1, '\r');
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	}
	//һ��һ�������ַ�
	USART_SendData(USART1, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
}
void uart1_send_byte (uint8_t ch)
{
	//USART1->SR;  //USART_GetFlagStatus(USART1, USART_FLAG_TC) �����һ���ַ�����ʧ�ܵ�����
	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	//һ��һ�������ַ�
	USART_SendData(USART1, (unsigned char) ch);
	//�ȴ��������
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
}

void uart1_puts(const char *s)
{
	while (*s) {
		uart1_send_data(*s++);
	}
}

void uart1_dma_config (void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//������DMA����    
    //����DMAʱ��  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
    //DMA1ͨ��5����  
    DMA_DeInit(DMA1_Channel5);  
    //�����ַ  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);  
    //�ڴ��ַ  
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)cmd_analyze.rec_buf1;  
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
    DMA_Init(DMA1_Channel5,&DMA_InitStructure);  
		
		 
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); //��������ж�

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_5_INT_PREEM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_5_INT_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_Init(&NVIC_InitStructure);
		
    //ʹ��ͨ��5  
    DMA_Cmd(DMA1_Channel5,ENABLE);  
    //����DMA��ʽ����  
    USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);  
}
//����1 DMA��ʽ�����ж�  
void DMA1_Channel4_IRQHandler(void)  
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif	
	if(DMA_GetITStatus(DMA1_IT_TC4)){
		//�����־λ  
		DMA_ClearFlag(DMA1_FLAG_TC4);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA1_IT_GL4); //���ȫ���жϱ�־ 
		USART_DMACmd(USART1,USART_DMAReq_Tx, DISABLE); //DISABLE����1��DMA����  
		DMA_Cmd(DMA1_Channel4, DISABLE);  
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
//����1 DMA��ʽ�������ж�
void DMA1_Channel5_IRQHandler(void)  
{  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(DMA_GetITStatus(DMA1_IT_TC5)){
		//�����־λ  
		DMA_ClearFlag(DMA1_FLAG_TC5);//���ͨ��4������ɱ�־
		DMA_ClearITPendingBit(DMA1_IT_GL5); //���ȫ���жϱ�־ 
		my_env.uart_receive_finished1 = 2;
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}
//

void start_uart1_dma (u32 _memBaseAddr, U16 _size)
{
	/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
	/* Write to DMA1 Channel4 CMAR */
	DMA1_Channel4->CMAR = _memBaseAddr;
	DMA_SetCurrDataCounter (DMA1_Channel4, _size);
	DMA_Cmd(DMA1_Channel4, ENABLE);  
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���1��DMA����   
}

void uart1_init (void)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART1_RX_INT_PREEM;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART1_RX_INT_SUB;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = UART1_BOUND_RATE;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж� 
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE); //�������ڿ����ж� 
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
	
	uart1_dma_config ();

}


void stop_uart1_receive (void)
{
	DMA_Cmd(DMA1_Channel5,DISABLE);
	USART_ITConfig(USART1,USART_IT_IDLE, DISABLE); //�������ڿ����ж� 
}
void start_uart1_receive (void)
{        
	//���ô������ݳ���  
	memset (cmd_analyze.rec_buf1, 0, CMD_BUF_LEN);
	DMA_Cmd(DMA1_Channel5,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel5, CMD_BUF_LEN);  
	DMA_Cmd(DMA1_Channel5,ENABLE);
	USART_ITConfig(USART1,USART_IT_IDLE, ENABLE); //�������ڿ����ж� 
}


#if EN_USART1_RX   //���ʹ���˽���
//����1�����ж�     
void USART1_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET){  
		//USART_ClearFlag(USART1,USART_IT_IDLE);  
		temp = USART1->SR;  
		temp = USART1->DR; //��USART_IT_IDLE��־  
		temp = temp;
		my_env.uart_receive_finished1 = 1;
	}   
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif 
} 
#endif
