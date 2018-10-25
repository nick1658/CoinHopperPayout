#include "main.h"



int main (void)
{
	delay_init();	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init ();
	uart2_init ();
	uart3_init ();
	uart4_init ();
	led_init();		  	//初始化与LED连接的硬件接口
	system_env_init ();
	
	//LED0 = 0;
	//LED1 = 0;
	my_println ("system start...");
	uart1_send_byte ('1');
	uart2_send_byte ('2');
	uart3_send_byte ('3');
	while (1){
		if (my_env.uart_receive_finished4 == 1){
			my_env.uart_receive_finished4 = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); 
			if ((cmd_analyze.rec_buf4[rec_count-1] == 0x0d) || 
					 ((cmd_analyze.rec_buf4[rec_count-2] == 0x0d) && (cmd_analyze.rec_buf4[rec_count-1] == 0x0a))){
				if ((cmd_analyze.rec_buf4[rec_count-2] == 0x0d) && (cmd_analyze.rec_buf4[rec_count-1] == 0x0a)){
					cmd_analyze.rec_buf4[rec_count-2] = 0;
				}
				cmd_analyze.rec_buf4[rec_count-1] = 0;
				LED2 = !LED2;
				run_command (cmd_analyze.rec_buf4, 0);
				start_uart4_receive ();
				my_println("OK");
			}else{
				if (isprint (cmd_analyze.rec_buf4[rec_count - 1])){
					uart1_send_data(cmd_analyze.rec_buf4[rec_count - 1]);	
				}
			}
		}
		if (my_env.uart_receive_finished1 == 2){
			my_env.uart_receive_finished1 = 0;	
			start_uart1_receive ();
			LED0 = !LED0;
		}
		if (my_env.uart_receive_finished2 == 2){
			my_env.uart_receive_finished2 = 0;	
			start_uart2_receive ();
			LED0 = !LED0;
		}
		if (my_env.uart_receive_finished3 == 2){
			my_env.uart_receive_finished3 = 0;	
			start_uart3_receive ();
			LED1 = !LED1;
		}
		if (my_env.uart_receive_finished4 == 2){
			my_env.uart_receive_finished4 = 0;	
			start_uart4_receive ();
			LED1 = !LED1;
		}
	}
//	while(1)
//	{
//		LED0 = 0;
//		LED1 = 1;
//		LED2 = 1;
//		delay_ms(100);	 //延时300ms
//		LED0 = 1;
//		LED1 = 0;
//		LED2 = 1;
//		delay_ms(100);	//延时300ms
//		LED0 = 1;
//		LED1 = 1;
//		LED2 = 0;
//		delay_ms(100);	//延时300ms
//	}
}

