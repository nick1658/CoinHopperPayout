#ifndef __USART5_H
#define __USART5_H

#define UART5_BOUND_RATE 9600
#define EN_UART5_RX 1

#define UART5_RECV_TIMEOUT 10
#define SPI3_RECV_TIMEOUT 10

void uart5_send_byte (uint8_t ch);
void uart5_send_data (int ch);
void uart5_init (void);
void uart5_puts(const char *s);
void stop_uart5_receive (void);
void start_uart5_receive (void);
void start_uart5_dma (uint32_t _memBaseAddr, uint16_t _size);

#endif


