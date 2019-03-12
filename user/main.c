#include "main.h"


#define HEADER_BYTE 0x05

#define RED_FLAG_SEND_TO_SLAVE 0x10
#define RED_FLAG_SEND_TO_MASTER  0x01

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

#define HOPPER_OFFLINE	 0x80

#define ONE_COIN_MSG 0x07
#define FINISH_MSG 0x08

#define ACK_MSG 0xAA
#define BUSY_MSG 0xBB


#define RED_FLAG_PAYOUT_BUF_LEN 6
#define HOPPER_NUM 3

#define UART4_SEND_BUF_NUM 64
#define UART_SEND_TIMEOUT 50


#define DISPENSE_TIMEOUT (12*1000)
#define COMMUNICATION_TIMEOUT (2*1000)

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
//	uint8_t hopper_offline_flag[HOPPER_NUM];
	void (*p_uart_send_byte[HOPPER_NUM])(uint8_t ch);
	uint16_t red_flag_operate_status;
	uint16_t hopper_dispense_fin_num;
	uint32_t dispense_timeout;
	uint32_t communication_timeout;
	//////////////////para area start/////////////////////
	uint32_t para_dispense_timeout;
	//////////////////para area end/////////////////////
}s_hopper_env;


typedef struct _uart_send_buf
{
	uint16_t buf_id;
	u_red_flag_frame frame_buf;
	struct _uart_send_buf * p_next_buf;
}s_uart_send_buf;
typedef struct
{
	uint16_t used_buf_ctr;
	uint16_t free_buf_ctr;
	uint32_t last_send_timeout;
	s_uart_send_buf * p_send_first_buf;
	s_uart_send_buf * p_send_last_buf;
	s_uart_send_buf * p_first_free_buf;
}s_uart_send_buf_head;

u_red_flag_frame *p_red_flag_frame;
s_hopper_env hopper_env;
s_uart_send_buf uart_send_buf[UART4_SEND_BUF_NUM];
s_uart_send_buf_head uart_send_buf_head;
uint8_t test_cmd[] = {0x05, 0x10, 0x02, 0x14, 0x01, 0x2c};
//init uart4 send buf struct
void init_uart4_send_buf (void)
{
	uint16_t i;
	s_uart_send_buf * p_tmp = (s_uart_send_buf*)0;
	uart_send_buf_head.last_send_timeout = 0;
	uart_send_buf_head.used_buf_ctr = 0;
	uart_send_buf_head.free_buf_ctr = UART4_SEND_BUF_NUM;
	uart_send_buf_head.p_first_free_buf = &uart_send_buf[0];
	uart_send_buf_head.p_send_first_buf = (s_uart_send_buf*)0;
	uart_send_buf_head.p_send_last_buf = (s_uart_send_buf*)0;
	p_tmp = &uart_send_buf[0];
	for (i = 0; i < UART4_SEND_BUF_NUM; i++){
		p_tmp->p_next_buf = &uart_send_buf[i];
		p_tmp = &uart_send_buf[i];
	}
}
//
s_uart_send_buf * get_free_uart_send_buf (void)
{
	static uint16_t buf_id = 0;
	s_uart_send_buf * p_tmp = (s_uart_send_buf*)0;
	if (uart_send_buf_head.free_buf_ctr > 0){
		p_tmp = uart_send_buf_head.p_first_free_buf;
		uart_send_buf_head.p_first_free_buf = p_tmp->p_next_buf;
		p_tmp->buf_id = ++buf_id;
		uart_send_buf_head.free_buf_ctr--;
	}
	return p_tmp;
}
//
void return_uart_send_buf (s_uart_send_buf * p_buf)
{
	p_buf->buf_id = 0;
	p_buf->p_next_buf = uart_send_buf_head.p_first_free_buf;
	uart_send_buf_head.p_first_free_buf = p_buf;
	uart_send_buf_head.free_buf_ctr++;
}
//
void insert_to_send_Q (s_uart_send_buf * p_buf)
{
	if (uart_send_buf_head.used_buf_ctr == 0){
		uart_send_buf_head.p_send_first_buf = p_buf;
		uart_send_buf_head.p_send_last_buf = p_buf;
	}else{
		uart_send_buf_head.p_send_last_buf->p_next_buf = p_buf;
		uart_send_buf_head.p_send_last_buf = p_buf;
	}
	p_buf->p_next_buf = 0;
	uart_send_buf_head.used_buf_ctr++;
}
//
void remove_first_item_from_send_Q (void)
{
	s_uart_send_buf * p_tmp;
	if (uart_send_buf_head.used_buf_ctr > 0){
		p_tmp = uart_send_buf_head.p_send_first_buf;
		uart_send_buf_head.p_send_first_buf = p_tmp->p_next_buf;
		uart_send_buf_head.used_buf_ctr--;
		if (uart_send_buf_head.used_buf_ctr == 0){
			uart_send_buf_head.p_send_last_buf = 0;
		}
		return_uart_send_buf(p_tmp);
	}
}
//
uint32_t send_to_uart_buf(uint8_t *buf)
{
	uint8_t i;	
	s_uart_send_buf * p_buf;
	p_buf = get_free_uart_send_buf ();
	if (p_buf > 0){
		for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN; i++){
			p_buf->frame_buf.fill[i] = buf[i];
		}
		insert_to_send_Q (p_buf);
		return 1;
	}else{
		return 0;
	}
}
//
void test_uart_send_buf (void)
{
	s_uart_send_buf * p_buf;
	/////////////////////////////////////////////////////////////////////////
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x05;
	insert_to_send_Q (p_buf);
	remove_first_item_from_send_Q ();
	////////////////////////////////////////////////////////////////////////////////////
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x05;
	insert_to_send_Q (p_buf);
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x06;
	insert_to_send_Q (p_buf);
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x07;
	insert_to_send_Q (p_buf);
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
	////////////////////////////////////////////////////////////////////////////////////
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x05;
	insert_to_send_Q (p_buf);
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x06;
	insert_to_send_Q (p_buf);
	p_buf = get_free_uart_send_buf ();
	p_buf->frame_buf.fill[0] = 0x07;
	insert_to_send_Q (p_buf);
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
	remove_first_item_from_send_Q ();
}

//TIM3 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
#define TIM3_ARR  999
#define TIM3_PSC  71
void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
	
   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM3_INT_PREEM;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIM3_INT_SUB;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}
//定时器3中断服务程序
static u16 tim3_count = 0;
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){ //检查指定的TIM中断发生与否:TIM 中断源 
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
		tim3_count++;
//////////////////////////////////////////////////////////////////////////////		
/////////////////////////////////////////////////////////////////////////////////		
	}
}



#define FILL_RED_FLAG_SLAVE_FRAME(BUF, ADDR, CMD, DATA) { \
	BUF[0] = HEADER_BYTE; \
	BUF[1] = RED_FLAG_SEND_TO_MASTER; \
	BUF[2] = ADDR; \
	BUF[3] = CMD; \
	BUF[4] = DATA; \
	BUF[5] = 0; \
	uint8_t i; \
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN - 1; i++){ \
		BUF[5] += BUF[i]; \
	} \
}

void send_to_uart1 (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++){ 
		uart1_send_byte (buf[i]); 
	}
}
void send_to_uart2 (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++){ 
		uart2_send_byte (buf[i]); 
	}
}
void send_to_uart3 (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++){ 
		uart3_send_byte (buf[i]); 
	}
}

void send_to_all_hopper_use_uart (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++){ 
		uart1_send_byte (buf[i]); 
		uart2_send_byte (buf[i]); 
		uart3_send_byte (buf[i]); 
	}
}
void send_to_uart4 (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++){ 
		uart4_send_byte (buf[i]); 
	}
}
void send_first_buf_data_use_uart()
{
	uint8_t send_dir = 0;
	send_dir = uart_send_buf_head.p_send_first_buf->frame_buf.data.dir;
	switch (send_dir)
	{
		case RED_FLAG_SEND_TO_MASTER:
			send_to_uart4 (&(uart_send_buf_head.p_send_first_buf->frame_buf.fill[0]), RED_FLAG_PAYOUT_BUF_LEN);
			break;
		case RED_FLAG_SEND_TO_SLAVE:
			send_to_all_hopper_use_uart (&(uart_send_buf_head.p_send_first_buf->frame_buf.fill[0]), RED_FLAG_PAYOUT_BUF_LEN);
//			send_to_uart1 (&(uart_send_buf_head.p_send_first_buf->frame_buf.fill[0]), RED_FLAG_PAYOUT_BUF_LEN);
//			send_to_uart2 (&(uart_send_buf_head.p_send_first_buf->frame_buf.fill[0]), RED_FLAG_PAYOUT_BUF_LEN);
//			send_to_uart3 (&(uart_send_buf_head.p_send_first_buf->frame_buf.fill[0]), RED_FLAG_PAYOUT_BUF_LEN);
			break;
		default: break;
	}
	remove_first_item_from_send_Q ();
	uart_send_buf_head.last_send_timeout = UART_SEND_TIMEOUT;
}
//
void test_red_flag (void)
{
	uint16_t i;
	//LED0 = 0;
	//LED1 = 0;
//	my_println ("system start...");
//	uart1_send_byte ('1');
//	uart2_send_byte ('2');
//	uart3_send_byte ('3');
//	i = '4';
//	do
//	{
//		delay_ms(100);
//		uart4_send_byte (i++);
//		if (i > '9')
//		i = '0';
//	}while (0);
	for (i = 0; i < sizeof (test_cmd); i++){
		uart1_send_byte (test_cmd[i]);
	}
	for (i = 0; i < sizeof (test_cmd); i++){
		uart2_send_byte (test_cmd[i]);
	}
	for (i = 0; i < sizeof (test_cmd); i++){
		uart3_send_byte (test_cmd[i]);
	}
}

void hopper_env_init (void)
{
	int i;
	S8 *p = (S8 *) &hopper_env;
	for (i = 0; i < sizeof(s_hopper_env); i++){
		*(p++) = 0;
	}
	hopper_env.p_uart_send_byte[0] = uart1_send_byte;//uart1_send_byte;
	hopper_env.p_uart_send_byte[1] = uart3_send_byte;//uart2_send_byte;
	hopper_env.p_uart_send_byte[2] = uart2_send_byte;//uart3_send_byte;
	hopper_env.hopper_dispense_fin_num = 3;
	hopper_env.para_dispense_timeout = DISPENSE_TIMEOUT;
}
//
void user_init (void)
{
	delay_init();	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init ();
	uart2_init ();
	uart3_init ();
	uart4_init ();
	TIM3_PWM_Init (TIM3_ARR, TIM3_PSC); //1ms
	hopper_env_init ();
	led_init();		  	//初始化与LED连接的硬件接口
	system_env_init ();
	init_uart4_send_buf ();
}
//
int red_flag_slave_res (uint8_t * buf, uint8_t len)
{
	uint8_t i;
	//calc checksum----------------------------------------------------------
	buf[5] = 0;
	for (i = 0; i < len - 1; i++){
		buf[i] = buf[i];
		buf[5] += buf[i];
	}	
	/////////////////////////////////////////////////////////////////////////
	send_to_uart_buf (buf);
	return 0;
}
//
int red_flag_hopper_res_process (u_red_flag_frame *p_frame)
{
	int i;
	uint8_t checksum = 0;
	uint8_t send_to_uart_flag = 0;
	for (i = 0; i < RED_FLAG_PAYOUT_BUF_LEN - 1; i++){
		checksum += p_frame->fill[i];
	}
	if (checksum == p_frame->data.checksum){
		send_to_uart_flag = 0;
		switch (p_frame->data.cmd)
		{
			case FINISH_MSG:
				hopper_env.hopper_dispense_fin_num++;
				hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
				if (hopper_env.hopper_dispense_fin_num >= 3){
					hopper_env.dispense_timeout = hopper_env.para_dispense_timeout;
				}
				send_to_uart_flag = 1;
				break;
			case ONE_COIN_MSG:
				hopper_env.dispense_timeout = DISPENSE_TIMEOUT;
				if (hopper_env.hopper_payout_num[p_frame->data.addr] > 0){
					hopper_env.hopper_payout_num[p_frame->data.addr]--;
				}
				break;
			case STATUS_MSG:
				if (((hopper_env.red_flag_req_flag[p_frame->data.addr] == PAYOUT_REQUEST_LIVE_MSG)
					|| (hopper_env.red_flag_req_flag[p_frame->data.addr] == EMPTY_HOPPER)
					) && p_frame->data.data > 0){
					hopper_env.hopper_dispense_fin_num++;
					if (hopper_env.hopper_dispense_fin_num >= 3){
						hopper_env.dispense_timeout = hopper_env.para_dispense_timeout;
					}
					p_frame->data.cmd = FINISH_MSG;
					p_frame->data.data = hopper_env.hopper_payout_num[p_frame->data.addr];
					hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
					send_to_uart_flag = 1;
				}else if ((hopper_env.red_flag_req_flag[p_frame->data.addr] == STATUS_REQUEST)){
					hopper_env.red_flag_req_flag[p_frame->data.addr] = 0;
					send_to_uart_flag = 1;
				}
				break;
			case ACK_MSG:
//				hopper_env.hopper_offline_flag[p_frame->data.addr] = 0;
				break;
			case BUSY_MSG:
				send_to_uart_flag = 1;
				break;
			default:break;
		}
		if (send_to_uart_flag == 1){
			red_flag_slave_res (p_frame->fill, RED_FLAG_PAYOUT_BUF_LEN);
		}
	}
	return 0;
}
//
void hopper_task (void)
{		
	uint32_t i = 0;
	if (my_env.uart_receive_finished1 == 1){
		my_env.uart_receive_finished1 = 0;	
		rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); 
		i = 0;
		while (i < rec_count){
			p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf1[i]);
			red_flag_hopper_res_process (p_red_flag_frame);
			i += RED_FLAG_PAYOUT_BUF_LEN;
		}
		start_uart1_receive ();
	}
	if (my_env.uart_receive_finished2 == 1){
		my_env.uart_receive_finished2 = 0;	
		rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel6); 
		i = 0;
		while (i < rec_count){
			p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf2[i]);
			red_flag_hopper_res_process (p_red_flag_frame);
			i += RED_FLAG_PAYOUT_BUF_LEN;
		}
		start_uart2_receive ();
	}
	if (my_env.uart_receive_finished3 == 1){
		my_env.uart_receive_finished3 = 0;	
		rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel3); 
		i = 0;
		while (i < rec_count){
			p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf3[i]);
			red_flag_hopper_res_process (p_red_flag_frame);
			i += RED_FLAG_PAYOUT_BUF_LEN;
		}
		start_uart3_receive ();
	}
}
//
int red_flag_master_msg_process (u_red_flag_frame *p_frame)
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
					if (p_frame->data.data > 0){
						if (hopper_env.hopper_dispense_fin_num > 0){
							hopper_env.hopper_dispense_fin_num--;
						}
					}
//					hopper_env.hopper_offline_flag[p_frame->data.addr] = HOPPER_OFFLINE;
					hopper_env.hopper_payout_num[p_frame->data.addr] = p_frame->data.data;
					hopper_env.red_flag_operate_status = PAYOUT_REQUEST_LIVE_MSG;
				}else {//参数配置
					switch (p_frame->data.addr)
					{
						case 0x80:
							hopper_env.para_dispense_timeout = p_frame->data.data*1000;
							break;
						default:break;
					}
				}
				break;
			case REMAINING_COINS_REQUEST:
				break;
			case STATUS_REQUEST:
				hopper_env.red_flag_operate_status = STATUS_REQUEST;
				break;
			case EMPTY_HOPPER:
				hopper_env.red_flag_operate_status = EMPTY_HOPPER;
				for (i = 0; i < HOPPER_NUM; i++){
					hopper_env.hopper_payout_num[i] = p_frame->data.data;
				}
				if (hopper_env.hopper_dispense_fin_num > 0){
					hopper_env.hopper_dispense_fin_num--;
				}
				break;
			case RESET_REQUEST:
				break;
			default:break;
		}
	}
	send_to_uart_buf (&(p_frame->fill[0]));
	return 0;
}
void dispense_task (void)
{
	uint32_t i = 0;
	uint8_t buf[RED_FLAG_PAYOUT_BUF_LEN];
	//找零操作
	if (my_env.uart_receive_finished4 == 1){
		//LED0_NOT;
		my_env.uart_receive_finished4 = 0;	
		rec_count = CMD_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Channel3); 
		i = 0;
		FILL_RED_FLAG_SLAVE_FRAME (buf, 0xFF, ACK_MSG, 0);
		send_to_uart_buf (buf);
		while (i < rec_count){
			p_red_flag_frame = (u_red_flag_frame*)&(cmd_analyze.rec_buf4[i]);
			red_flag_master_msg_process (p_red_flag_frame);
			i += RED_FLAG_PAYOUT_BUF_LEN;
		}
		start_uart4_receive ();
	}
}
void fin_dispense_op (void)
{
	uint8_t buf[RED_FLAG_PAYOUT_BUF_LEN];
	FILL_RED_FLAG_SLAVE_FRAME (buf, 0xFF, FINISH_MSG, 0);
	if (send_to_uart_buf(buf) == 0){//返回0，表示发送不成功
		hopper_env.dispense_timeout = 200;//200ms 后重试
	}
	
	hopper_env.hopper_dispense_fin_num = HOPPER_NUM;
	LED0 = 0;
}
//
void fin_res_status_op (void)
{
	uint8_t buf[RED_FLAG_PAYOUT_BUF_LEN];
	FILL_RED_FLAG_SLAVE_FRAME (buf, 0x80, FINISH_MSG, 0);
	if (send_to_uart_buf(buf) == 0){//返回0，表示发送不成功
		hopper_env.dispense_timeout = 200;//200ms 后重试
	}
	hopper_env.hopper_dispense_fin_num = HOPPER_NUM;
	LED0 = 0;
}
//
void timer_1ms_task (void)
{
	if (hopper_env.dispense_timeout > 0){
		hopper_env.dispense_timeout--;
		if (hopper_env.dispense_timeout == 0){
			fin_dispense_op ();
		}
	}
	if (hopper_env.communication_timeout > 0){
		hopper_env.communication_timeout--;
		if (hopper_env.communication_timeout == 0){
			fin_res_status_op ();
		}
	}
	if ((hopper_env.red_flag_operate_status == PAYOUT_REQUEST_LIVE_MSG) ||
			(hopper_env.red_flag_operate_status == EMPTY_HOPPER)){
		hopper_env.red_flag_operate_status = 0;
		hopper_env.dispense_timeout = DISPENSE_TIMEOUT;
	}
	if ((hopper_env.red_flag_operate_status == STATUS_REQUEST)){
		hopper_env.red_flag_operate_status = 0;
		hopper_env.communication_timeout = COMMUNICATION_TIMEOUT;
	}
	if (uart_send_buf_head.last_send_timeout > 0){
		uart_send_buf_head.last_send_timeout--;
	}
}

void timer_500ms_task (void)
{
//	uint8_t buf[RED_FLAG_PAYOUT_BUF_LEN];
	if (hopper_env.dispense_timeout > 0){
		LED0_NOT;
		//FILL_RED_FLAG_SLAVE_FRAME (buf, 0xFF, ACK_MSG, 0);
		//send_to_uart_buf (buf);
	}
	if (hopper_env.communication_timeout > 0){
		LED0_NOT;
	}
}
//
void timer_task (uint32_t time )
{	
	static uint32_t sys_run_time;
	static uint32_t sys_time_ctr;
	if (sys_run_time != time){//1ms 执行一次的任务
		timer_1ms_task ();
		if ((sys_run_time % 500 == 0)){//500ms 执行一次的任务
			timer_500ms_task ();
		}
		/////////////////////////////////////////////
		LED1 = (sys_time_ctr < 20) ? 1: 0;
		sys_time_ctr++;
		if (sys_time_ctr > 1000){
			sys_time_ctr = 0;
		}
		////////////////////////////////////////////
	}
	////////////////////////////////////////////////
	sys_run_time = time;
}
void uart_send_task (void)
{
	if (uart_send_buf_head.last_send_timeout == 0){
		if (uart_send_buf_head.used_buf_ctr > 0){
			send_first_buf_data_use_uart ();
		}
	}
}
//
void exception_task (void)
{	
	if (my_env.uart_receive_finished1 == 2){
		my_env.uart_receive_finished1 = 0;	
		start_uart1_receive ();
	}
	if (my_env.uart_receive_finished2 == 2){
		my_env.uart_receive_finished2 = 0;	
		start_uart2_receive ();
	}
	if (my_env.uart_receive_finished3 == 2){
		my_env.uart_receive_finished3 = 0;	
		start_uart3_receive ();
	}
	if (my_env.uart_receive_finished4 == 2){
		my_env.uart_receive_finished4 = 0;	
		start_uart4_receive ();
	}
}
//
int main (void)
{
	user_init ();
//	test_uart_send_buf ();
//	test_red_flag ();
	while (1){
		///////////////////////////////////////////////////////////////////////////////
		hopper_task ();
		////////////////////////////////////////////////////////////////////////////////
		timer_task (tim3_count);
		////////////////////////////////////////////////////////////////////////////////
		dispense_task ();
		///////////////////////////////////////////////////////////////////////////////
		uart_send_task ();
		///////////////////////////////////////////////////////////////////////////////
		exception_task ();
		///////////////////////////////////////////////////////////////////////////////
	}
}

