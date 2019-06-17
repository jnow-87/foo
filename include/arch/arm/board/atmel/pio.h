/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ATMEL_PIO_H
#define ATMEL_PIO_H


#include <sys/types.h>


/* types */
typedef enum{
	PIO_A = 0,
	PIO_B,
	PIO_C,
	PIO_D,
	PIO_E,
} pio_port_t;

typedef enum{
	PIO_FUNC_A = 0,
	PIO_FUNC_B = 1,
	PIO_FUNC_C = 2,
	PIO_FUNC_D = 3
} pio_func_t;


/* prototypes */
void pio_pin_enable(pio_port_t port, uint8_t pin, pio_func_t func);
void pio_pin_disable(pio_port_t port, uint8_t pin, pio_func_t func);
void pio_pin_set(pio_port_t port, uint8_t pin);
void pio_pin_clear(pio_port_t port, uint8_t pin);


#endif // ATMEL_PIO_H
