#ifndef __USART4_H
#define __USART4_H

#define UART4_BOUND_RATE 9600
#define EN_UART4_RX 1

void uart4_send_byte (uint8_t ch);
void uart4_send_data (int ch);
void uart4_init (void);
void uart4_puts(const char *s);
void stop_uart4_receive (void);
void start_uart4_receive (void);
void start_uart4_dma (uint32_t _memBaseAddr, uint16_t _size);

#endif


