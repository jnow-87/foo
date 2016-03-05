#ifndef AVR_UART_H
#define AVR_UART_H


#include <sys/error.h>


/* prototypes */
error_t avr_putchar(char c);
error_t avr_puts(const char* s);


#endif // AVR_UART_H
