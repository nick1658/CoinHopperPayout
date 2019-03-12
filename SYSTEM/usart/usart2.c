#include "main.h"


void uart2_send_data (int ch)
{
	//USART2->SR;  //USART_GetFlagStatus(USART2, USART_FLAG_TC) 解决第一个字符发送失败的问题
	
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(USART2, '\r');
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
	}
	//一个一个发送字符
	USART_SendData(USART2, (unsigned char) ch);
	//等待发送完成
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
}
void uart2_send_byte (uint8_t ch)
{
	//USART2->SR;  //USART_GetFlagStatus(USART2, USART_FLAG_TC) 解决第一个字符发送失败的问题
	
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
	//一个一个发送字符
	USART_SendData(USART2, (unsigned char) ch);
	//等待发送完成
//	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
}

void uart2_puts(const char *s)
{
	while (*s) {
		uart2_send_data(*s++);
	}
}

void uart2_dma_config (void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//串口收DMA配置    
    //启动DMA时钟  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
    //DMA1通道5配置  
    DMA_DeInit(DMA1_Channel6);  
    //外设地址  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);  
    //内存地址  
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)cmd_analyze.rec_buf2;  
    //dma传输方向单向  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
    //设置DMA在传输时缓冲区的长度  
    DMA_InitStructure.DMA_BufferSize = CMD_BUF_LEN;  
    //设置DMA的外设递增模式，一个外设  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
    //设置DMA的内存递增模式  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
    //外设数据字长  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
    //内存数据字长  
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
    //设置DMA的传输模式  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
    //设置DMA的优先级别  
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
    //设置DMA的2个memory中的变量互相访问  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
    DMA_Init(DMA1_Channel6,&DMA_InitStructure);  
		
		 
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE); //传输结束中断

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_5_INT_PREEM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_5_INT_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_Init(&NVIC_InitStructure);
		
    //使能通道6  
    DMA_Cmd(DMA1_Channel6,ENABLE);  
    //采用DMA方式接收  
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  
}
//串口1 DMA方式发送中断  
void DMA1_Channel7_IRQHandler(void)  
{
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif	
	if(DMA_GetITStatus(DMA1_IT_TC7)){
		//清除标志位  
		DMA_ClearFlag(DMA1_FLAG_TC7);//清除通道4传输完成标志
		DMA_ClearITPendingBit(DMA1_IT_GL7); //清除全部中断标志 
		USART_DMACmd(USART2,USART_DMAReq_Tx, DISABLE); //DISABLE串口1的DMA发送  
		DMA_Cmd(DMA1_Channel7, DISABLE);  
	}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
//串口1 DMA方式接收满中断
void DMA1_Channel6_IRQHandler(void)  
{  
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(DMA_GetITStatus(DMA1_IT_TC6)){
		//清除标志位  
		DMA_ClearFlag(DMA1_FLAG_TC6);//清除通道4传输完成标志
		DMA_ClearITPendingBit(DMA1_IT_GL6); //清除全部中断标志 
		my_env.uart_receive_finished2 = 2;
	}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
//

void start_uart2_dma (u32 _memBaseAddr, U16 _size)
{
	/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
	/* Write to DMA1 Channel4 CMAR */
	DMA1_Channel7->CMAR = _memBaseAddr;
	DMA_SetCurrDataCounter (DMA1_Channel7, _size);
	DMA_Cmd(DMA1_Channel7, ENABLE);  
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); //使能串口1的DMA发送   
}

void uart2_init (void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
  
	//USART2_TX   GPIOA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
   
	//USART2_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA.3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART2_RX_INT_PREEM;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART2_RX_INT_SUB;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = UART2_BOUND_RATE;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断 
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE); //开启串口空闲中断 
	USART_Cmd(USART2, ENABLE);                    //使能串口2 
	
	uart2_dma_config ();

}


void stop_uart2_receive (void)
{
	DMA_Cmd(DMA1_Channel6,DISABLE);
	USART_ITConfig(USART2,USART_IT_IDLE, DISABLE); //开启串口空闲中断 
}
void start_uart2_receive (void)
{        
	//设置传输数据长度  
	memset (cmd_analyze.rec_buf2, 0, CMD_BUF_LEN);
	DMA_Cmd(DMA1_Channel6,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel6, CMD_BUF_LEN);  
	DMA_Cmd(DMA1_Channel6,ENABLE);
	USART_ITConfig(USART2,USART_IT_IDLE, ENABLE); //开启串口空闲中断 
}


#if EN_USART2_RX   //如果使能了接收
//串口1接收中断     
void USART2_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET){  
		//USART_ClearFlag(USART2,USART_IT_IDLE);  
		temp = USART2->SR;  
		temp = USART2->DR; //清USART_IT_IDLE标志  
		temp = temp;
		my_env.uart_receive_finished2 = 1;
	}   
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif 
} 
#endif
