#include "main.h"

s_ad8804_env ad8804_env;
uint8_t sin_value[] = {
	128,131,134,137,140,143,146,149,
	152,155,158,162,165,167,170,173,
	176,179,182,185,188,190,193,196,
	198,201,203,206,208,211,213,215,
	218,220,222,224,226,228,230,232,
	233,235,237,238,240,241,243,244,
	245,246,247,248,249,250,251,252,
	252,253,253,254,254,254,254,254,
	254,254,254,254,254,253,253,252,
	252,251,250,250,249,248,247,246,
	244,243,242,240,239,237,236,234,
	232,231,229,227,225,223,221,219,
	216,214,212,210,207,205,202,200,
	197,194,192,189,186,183,181,178,
	175,172,169,166,163,160,157,154,
	151,148,145,142,138,135,132,129,
	126,123,120,117,113,110,107,104,
	101,98,95,92,89,86,83,80,
	77,74,72,69,66,63,61,58,
	55,53,50,48,45,43,41,39,
	36,34,32,30,28,26,24,23,
	21,19,18,16,15,13,12,11,
	9,8,7,6,5,5,4,3,
	3,2,2,1,1,1,1,1,
	1,1,1,1,1,2,2,3,
	3,4,5,6,7,8,9,10,
	11,12,14,15,17,18,20,22,
	23,25,27,29,31,33,35,37,
	40,42,44,47,49,52,54,57,
	59,62,65,67,70,73,76,79,
	82,85,88,90,93,97,100,103,
	106,109,112,115,118,121,124
};


int main (void)
{
	uint8_t i;
	delay_init();	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init ();
	ad8804_init();		  	//初始化与LED连接的硬件接口
	system_env_init ();
	ad8804_env.da_addr[0] = DA1;
	ad8804_env.da_addr[1] = DA2;
	ad8804_env.da_addr[2] = DA3;
	ad8804_env.da_addr[3] = DA4;
	ad8804_env.da_addr[4] = DA5;
	ad8804_env.da_addr[5] = DA6;
	ad8804_env.da_addr[6] = DA7;
	ad8804_env.da_addr[7] = DA8;
	ad8804_env.da_addr[8] = DA9;
	ad8804_env.da_addr[9] = DA10;
	ad8804_env.da_addr[10] = DA11;
	ad8804_env.da_addr[11] = DA12;
	SHDL = 1;
	SPI_CS = 1;
	SPI_SCK = 1;
	SPI_MOSI = 1;
	
	LED0 = 0;
	LED1 = 0;
	LED2 = 0;
	my_println ("system start...");
	for (i = 0; i < 12; i++){
		ad8804_write_ch (ad8804_env.da_addr[i], 0);
	}
	while (1){
		if (my_env.uart_receive_finished == 1){
			my_env.uart_receive_finished = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); 
			if ((cmd_analyze.rec_buf[rec_count-1] == 0x0d) || 
					 ((cmd_analyze.rec_buf[rec_count-2] == 0x0d) && (cmd_analyze.rec_buf[rec_count-1] == 0x0a))){
				if ((cmd_analyze.rec_buf[rec_count-2] == 0x0d) && (cmd_analyze.rec_buf[rec_count-1] == 0x0a)){
					cmd_analyze.rec_buf[rec_count-2] = 0;
				}
				cmd_analyze.rec_buf[rec_count-1] = 0;
				LED2 = !LED2;
				run_command (cmd_analyze.rec_buf, 0);
				start_uart1_receive ();
				my_println("OK");
			}else{
				if (isprint (cmd_analyze.rec_buf[rec_count - 1])){
					uart1_send_data(cmd_analyze.rec_buf[rec_count - 1]);	
				}
			}
		}else if (my_env.uart_receive_finished == 2){
			my_env.uart_receive_finished = 0;	
			start_uart1_receive ();
			LED0 = !LED0;
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

