#include "main.h"


void uart5_send_data (int ch)
{
	//UART5->SR;  //USART_GetFlagStatus(UART5, USART_FLAG_TC) 解决第一个字符发送失败的问题
	
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);	
	if(ch == '\n'){
		USART_SendData(UART5, '\r');
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	}
	//一个一个发送字符
	USART_SendData(UART5, (unsigned char) ch);
	//等待发送完成
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
}
void uart5_send_byte (uint8_t ch)
{
	//UART5->SR;  //USART_GetFlagStatus(UART5, USART_FLAG_TC) 解决第一个字符发送失败的问题
	
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)!=SET);
	//一个一个发送字符
	USART_SendData(UART5, (unsigned char) ch);
	//等待发送完成
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
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//使能UART5时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PC端口时钟
  
	//UART5_TX   GPIOC.12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.12
   
	//UART5_RX	  GPIOD.2初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PC.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOC.11
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5,DISABLE);

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART5_RX_INT_PREEM;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART5_RX_INT_SUB;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = UART5_BOUND_RATE;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(UART5, &USART_InitStructure); //初始化串口5
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启串口接受中断 
	//USART_ITConfig(UART5,USART_IT_IDLE,ENABLE); //开启串口空闲中断 
	USART_Cmd(UART5, ENABLE);                    //使能串口5 

}



#if EN_UART5_RX   //如果使能了接收
//串口1接收中断     
void UART5_IRQHandler(void)                                 
{     
    uint32_t temp = 0;  
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif    
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET){  
		//USART_ClearFlag(UART5,USART_IT_IDLE);  
		temp = UART5->SR;  
		temp = UART5->DR; //清USART_IT_RXNE标志  
		cmd_analyze.uart5_ctr %=CMD_BUF_LEN;
		cmd_analyze.rec_buf5[cmd_analyze.uart5_ctr++] = temp;
		cmd_analyze.uart5_recv_time_out = UART5_RECV_TIMEOUT;
	}   
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif 
} 
#endif
