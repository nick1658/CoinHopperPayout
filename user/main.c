#include "main.h"

//命令
#define PAYOUT_REQUEST_NO_MSG 0x10
#define STATUS_REQUEST 0x11
#define RESET_REQUEST 0x12
#define REMAINING_COINS_REQUEST 0x13
#define PAYOUT_REQUEST_LIVE_MSG 0x14
#define EMPTY_HOPPER 0x15
//响应说明
#define STATUS_MSG 0x04
#define STATUS_MSG_MOTOR_ERROR 0x01
#define STATUS_MSG_HOPPER_LOW 0x02
#define STATUS_MSG_PRISM_FAILED 0x08
#define STATUS_MSG_SHAFT_FAILED 0x10
#define STATUS_MSG_BUSY        0x20

#define ONE_COIN_MSG 0x07
#define FINISH_MSG 0x08

#define ACK_MSG 0xAA
#define BUSY_MSG 0xBB


#define RED_FLAG_PAYOUT_BUF_LEN 6
#define HOPPER_NUM 3

typedef struct
{
	uint8_t header;
	uint8_t dir;
	uint8_t addr;
	uint8_t cmd;
	uint8_t data;
	uint8_t checksum;
}s_red_flag_frame;

typedef union {
	uint8_t fill[RED_FLAG_PAYOUT_BUF_LEN];
	s_red_flag_frame data;
}u_red_flag_frame;

typedef struct
{
	uint8_t hopper_payout_num[HOPPER_NUM];
	uint8_t red_flag_req_flag[HOPPER_NUM];
	void (*p_uart_send_byte[HOPPER_NUM])(uint8_t ch);
}s_hopper_env;

u_red_flag_frame *p_red_flag_frame;
s_hopper_env hopper_env;


uint8_t test_cmd[] = {0x05, 0x10, 0x02, 0x14, 0x01, 0x2c};

#define SEND_RED_FLAG_FRAME(ADDR, CMD, DATA) { \
	red_flag_payout_buf[0] = 0x05; \
	red_flag_payout_buf[1] = 0x01; \
	red_flag_payout_buf[2] = ADDR; \
	red_flag_payout_buf[3] = CMD; \
	red_flag_payout_buf[4] = DATA; \
	red_flag_payout_buf[5] = 0; \
	uint8_t i; \
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN - 1; i++){ \
		red_flag_payout_buf[5] += red_flag_payout_buf[i]; \
	} \
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN; i++){ \
		Uart3_sendchar (red_flag_payout_buf[i]); \
	} \
}
int red_flag_frame_process (u_red_flag_frame *p_frame)
{
	int i;
	uint8_t checksum = 0;
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN - 1; i++){
		checksum += p_frame->fill[i];
	}
	if (checksum == p_frame->data.checksum){
		if (p_frame->data.addr < HOPPER_NUM){
			hopper_env.red_flag_req_flag[p_frame->data.addr] = p_frame->data.cmd;
		}
		switch (p_frame->data.cmd)
		{
			case PAYOUT_REQUEST_LIVE_MSG:
				if (p_frame->data.addr < HOPPER_NUM){
					hopper_env.hopper_payout_num[p_frame->data.addr] = p_frame->data.data;
				}
				break;
			case REMAINING_COINS_REQUEST:
				break;
			case STATUS_REQUEST:
				break;
			case EMPTY_HOPPER:
				break;
			case RESET_REQUEST:
				break;
			default:break;
		}
	}

	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN; i++){ 
		hopper_env.p_uart_send_byte[p_frame->data.addr](p_frame->fill[i]); 
	} 
	return 0;
}
//

int red_flag_slave_res (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	buf[5] = 0;
	for (i = 0; i < len - 1; i++){
		buf[5] += buf[i];
	}
	for (i = 0; i < len; i++){ 
		uart4_send_byte (buf[i]); 
	}
	delay_ms(20);	 //延时20ms 
	return 0;
}

int red_flag_hopper_res (u_red_flag_frame *p_frame)
{
	int i;
	uint8_t checksum = 0;
	uint8_t send_to_master = 0;
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN - 1; i++){
		checksum += p_frame->fill[i];
	}
	if (checksum == p_frame->data.checksum){
		send_to_master = 0;
		if (p_frame->data.cmd == FINISH_MSG){
			send_to_master = 1;
		}else if (p_frame->data.cmd == ONE_COIN_MSG){
			if (hopper_env.hopper_payout_num[p_frame->data.addr] > 0){
				hopper_env.hopper_payout_num[p_frame->data.addr]--;
			}
		}else if (p_frame->data.cmd == STATUS_MSG){
			if ((hopper_env.red_flag_req_flag[p_frame->data.addr] == PAYOUT_REQUEST_LIVE_MSG) && p_frame->data.data > 0){
				p_frame->data.cmd = FINISH_MSG;
				p_frame->data.data = hopper_env.hopper_payout_num[p_frame->data.addr];
				hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
				send_to_master = 1;
			}else if ((hopper_env.red_flag_req_flag[p_frame->data.addr] == STATUS_REQUEST)){
				hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
				send_to_master = 1;
			}
		}else if (p_frame->data.cmd == ACK_MSG){
			if (hopper_env.red_flag_req_flag[p_frame->data.addr] == 1){
				hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
				send_to_master = 1;
			}
		}else if (p_frame->data.cmd == BUSY_MSG){
			send_to_master = 1;
		}
		if (send_to_master == 1){
			red_flag_slave_res (p_frame->fill, RED_FLAG_PAYOUT_BUF_LEN);
		}
	}
	return 0;
}
//



void hopper_env_init (void)
{
	int i;
	S8 *p = (S8 *) &hopper_env;
	for (i = 0; i < sizeof(s_hopper_env); i++){
		*(p++) = 0;
	}
	hopper_env.p_uart_send_byte[0] = uart1_send_byte;
	hopper_env.p_uart_send_byte[1] = uart2_send_byte;
	hopper_env.p_uart_send_byte[2] = uart3_send_byte;
}

int main (void)
{
	uint32_t i = 0;
	delay_init();	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init ();
	uart2_init ();
	uart3_init ();
	uart4_init ();
	hopper_env_init ();
	led_init();		  	//初始化与LED连接的硬件接口
	system_env_init ();
	
	//LED0 = 0;
	//LED1 = 0;
//	my_println ("system start...");
//	uart1_send_byte ('1');
//	uart2_send_byte ('2');
//	uart3_send_byte ('3');
	i = '4';
	do
	{
		delay_ms(100);
		uart4_send_byte (i++);
		if (i > '9')
		i = '0';
	}while (0);
//	for (i = 0; i < sizeof (test_cmd); i++){
//		uart1_send_byte (test_cmd[i]);
//	}
//	for (i = 0; i < sizeof (test_cmd); i++){
//		uart2_send_byte (test_cmd[i]);
//	}
//	for (i = 0; i < sizeof (test_cmd); i++){
//		uart3_send_byte (test_cmd[i]);
//	}
	while (1){

		///////////////////////////////////////////////////////////////////////////////
		if (my_env.uart_receive_finished1 == 1){
			my_env.uart_receive_finished1 = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); 
			i = 0;
			while (i < rec_count){
				p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf1[i]);
				red_flag_hopper_res (p_red_flag_frame);
				i += RED_FLAG_PAYOUT_BUF_LEN;
			}
			start_uart1_receive ();
			LED1 = !LED1;
		}
		if (my_env.uart_receive_finished2 == 1){
			my_env.uart_receive_finished2 = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel6); 
			i = 0;
			while (i < rec_count){
				p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf2[i]);
				red_flag_hopper_res (p_red_flag_frame);
				i += RED_FLAG_PAYOUT_BUF_LEN;
			}
			start_uart2_receive ();
			LED1 = !LED1;
		}
		if (my_env.uart_receive_finished3 == 1){
			my_env.uart_receive_finished3 = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel3); 
			i = 0;
			while (i < rec_count){
				p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf3[i]);
				red_flag_hopper_res (p_red_flag_frame);
				i += RED_FLAG_PAYOUT_BUF_LEN;
			}
			start_uart3_receive ();
			LED1 = !LED1;
		}
		////////////////////////////////////////////////////////////////////////////////
		//找零操作
		if (my_env.uart_receive_finished4 == 1){
			my_env.uart_receive_finished4 = 0;	
			rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Channel3); 
			i = 0;
			while (i < rec_count){
				p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf4[i]);
				red_flag_frame_process (p_red_flag_frame);
				i += RED_FLAG_PAYOUT_BUF_LEN;
			}
			start_uart4_receive ();
			LED1 = !LED1;
		}
		///////////////////////////////////////////////////////////////////////////////
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

