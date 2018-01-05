#ifndef DRIVER_AVR_UART_H
#define DRIVER_AVR_UART_H


#include <sys/uart.h>


/* prototypes */
int avr_uart_config(unsigned int uart, uart_t *cfg);
char avr_uart_putchar(char c);
int avr_uart_puts(char const *s);
int avr_uart_putsn(unsigned int uart, char const *s, size_t n);



#endif // DRIVER_AVR_UART_H
