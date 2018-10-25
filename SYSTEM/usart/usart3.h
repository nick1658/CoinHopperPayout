#ifndef __USART3_H
#define __USART3_H

#define UART3_BOUND_RATE 9600
#define EN_USART3_RX 1

void uart3_send_byte (uint8_t ch);
void uart3_send_data (int ch);
void uart3_init (void);
void uart3_puts(const char *s);
void stop_uart3_receive (void);
void start_uart3_receive (void);
void start_uart3_dma (uint32_t _memBaseAddr, uint16_t _size);


#endif


