#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <stddef.h>

extern void (*uart_rx_cb)(uint8_t, char);

extern char uart1_rx_data;
extern int uart1_rx_flag;

extern char uart2_rx_data;
extern int uart2_rx_flag;

extern char uart3_rx_data;
extern int uart3_rx_flag;

extern char uart4_rx_data;
extern int uart4_rx_flag;


void UART_Init(uint8_t port, uint8_t gpio_port, uint16_t tx_pin, uint16_t rx_pin);
void UART_SetBaudRate(uint8_t port, uint32_t baud);
void UART_Enable(uint8_t port, uint8_t interrupt_priority);
void UART_TransmitString(uint8_t port, char* msg);
void UART_TransmitChar(uint8_t port, char c);
void UART_TransmitHex(uint8_t port, size_t num, size_t size);
void UART_TransmitInt(uint8_t port, size_t num, size_t size);

#endif