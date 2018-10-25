#ifndef _MY_CMD_
#define _MY_CMD_



#define REGISTERED 0x5AA5FF11
#define NOT_REGISTERED 0x12345678

#define CTRL_C 0X03


#define HEX_TO_DEC(X) (X >> 4) * 10 + (X & 0xf)

enum 
{
	TTY_IDLE = 0,
	TTY_CONSOLE,
	TTY_MODBUS
};

/*
 * Monitor Command Table
 */
 
#define CFG_CBSIZE	64		/* Console I/O Buffer Size	*/
 #define CFG_MAXARGS		16	/* max number of command args   */
#define CONFIG_CBSIZE					64		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_CBSIZE				64		/* Console I/O Buffer Size	*/
#define CONFIG_MAXARGS					16		/* max number of command args	*/
#define CONFIG_SYS_MAXARGS				16		/* max number of command args	*/
#define CFG_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_LONGHELP
//#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */

struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmdhandle)(struct cmd_tbl_s *, int, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CFG_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]);
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;


extern cmd_tbl_t  __u_boot_cmd_start;
extern cmd_tbl_t  __u_boot_cmd_end;

#define Struct_Section __attribute__ ((unused, section("my_cmd_sec")))


#define MY_CMD(name,maxargs,rep,cmd,usage,help) \
 __align(4) cmd_tbl_t __my_cmd_##name Struct_Section = {#name, maxargs, rep, cmd, usage, help}
 
extern cmd_tbl_t __my_cmd_task_manager;


typedef struct 
{  
	char const *cmd_name;                        //�����ַ���  
	int32_t max_args;                            //��������Ŀ  
	void (*handle)(int argc, void * cmd_arg);     //����ص�����  
	char  *help;                                 //������Ϣ  
}cmd_list_struct; 


#define ARG_NUM     8          //�����������Ĳ�������  
#define CMD_LEN     20         //������ռ�õ�����ַ�����  
#define CMD_BUF_LEN 60         //��������󳤶�  
       
typedef struct 
{  
	char rec_buf[CMD_BUF_LEN];            //�����������  
	char processed_buf[CMD_BUF_LEN];      //�洢�ӹ��������(ȥ�������ַ�)  
	int32_t cmd_arg[ARG_NUM];             //��������Ĳ���  
}cmd_analyze_struct;  


typedef struct
{
	U16 uart_receive_finished;
}s_system_env;

typedef struct
{
	cmd_tbl_t * cmdtp;     //����ص�����  
	int argc;
	void * cmd_arg;
}s_task_parameter;

extern s_system_env my_env;
extern cmd_analyze_struct cmd_analyze; 
extern uint32_t rec_count;

void system_env_init (void);
void print_ng_data (S16 index);
void print_system_env_info (void);
void fill_rec_buf(char data);
void vTaskCmdAnalyze( void );
u32 GetLockCode(char *id);

int run_command (const char *cmd, int flag);
int cmd_usage(cmd_tbl_t *cmdtp);

cmd_tbl_t *find_cmd (const char *cmd);

S16 is_repeate (S16 _coin_index);//  �б� �ǲ����رҵĺ���



extern unsigned int Image$$MY_CMD_RO_SEC$$Base;
extern unsigned int Image$$MY_CMD_RO_SEC$$Length;

#define MY_CMD_RO_SEC_LENGTH 		(&Image$$MY_CMD_RO_SEC$$Length)
#define MY_CMD_RO_SEC_START 		(&Image$$MY_CMD_RO_SEC$$Base)
#define MY_CMD_RO_SEC_END 			((unsigned int)(&Image$$MY_CMD_RO_SEC$$Base) + (unsigned int)MY_CMD_RO_SEC_LENGTH)

#endif