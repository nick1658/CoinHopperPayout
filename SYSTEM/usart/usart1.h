#ifndef __USART1_H
#define __USART1_H

#define UART1_BOUND_RATE 9600
#define EN_USART1_RX 1

void uart1_send_byte (uint8_t ch);
void uart1_send_data (int ch);
void uart1_init (void);
void uart1_puts(const char *s);
void stop_uart1_receive (void);
void start_uart1_receive (void);
void start_uart1_dma (uint32_t _memBaseAddr, uint16_t _size);


#define my_putc uart1_send_data
#define my_puts uart1_puts

#endif


