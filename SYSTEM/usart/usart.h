#ifndef __USART_H
#define __USART_H


#define UART1_BOUND_RATE 115200
#define EN_USART1_RX 1

#define UART3_BOUND_RATE 115200 //19200
#define UART3_BUF_LEN 64
#define EN_USART3_RX 1

void uart1_send_byte (uint8_t ch);
void uart1_send_data (int ch);
void uart1_init (void);
void uart1_puts(const char *s);
void stop_uart1_receive (void);
void start_uart1_receive (void);
void start_uart1_dma (uint32_t _memBaseAddr, uint16_t _size);

void uart3_init (void);
void stop_uart3_receive (void);
void start_uart3_receive (void);
void start_uart3_dma (uint32_t _memBaseAddr, uint16_t _size);


#define my_putc uart1_send_data
#define my_puts uart1_puts

extern unsigned char uart3_rec_buf[UART3_BUF_LEN];
extern uint16_t uart3_rec_count;
#endif


