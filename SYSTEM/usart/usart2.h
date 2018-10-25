#ifndef __USART2_H
#define __USART2_H

#define UART2_BOUND_RATE 9600
#define EN_USART2_RX 1

void uart2_send_byte (uint8_t ch);
void uart2_send_data (int ch);
void uart2_init (void);
void uart2_puts(const char *s);
void stop_uart2_receive (void);
void start_uart2_receive (void);
void start_uart2_dma (uint32_t _memBaseAddr, uint16_t _size);


#endif


