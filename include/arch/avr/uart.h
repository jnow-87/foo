#ifndef AVR_UART_H
#define AVR_UART_H


#include <sys/errno.h>


/* prototypes */
char avr_putchar(char c);
int avr_puts(char const *s);


#endif // AVR_UART_H
