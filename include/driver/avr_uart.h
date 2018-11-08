/*
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_AVR_UART_H
#define DRIVER_AVR_UART_H


/* prototypes */
char avr_uart_putchar(char c);
int avr_uart_puts(char const *s);


#endif // DRIVER_AVR_UART_H
