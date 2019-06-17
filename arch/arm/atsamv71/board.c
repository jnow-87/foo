/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/atmel/pmc.h>
#include <arch/arm/atmel/eefc.h>
#include <arch/arm/atmel/pio.h>


/* global functions */
int atsamv71_init(void){
	int r;


	r = 0;

	/* init clocks */
	r |= eefc_init();
	r |= pmc_init();

	/* init I/O pins */
	pio_pin_enable(PIO_A, 23, PIO_FUNC_C);	// led0
	pio_pin_enable(PIO_C, 9, PIO_FUNC_A);	// led1
	pio_pin_enable(PIO_B, 0, PIO_FUNC_C);	// uart0 rx
	pio_pin_enable(PIO_B, 1, PIO_FUNC_C);	// uart0 tx
	pio_pin_enable(PIO_A, 21, PIO_FUNC_A);	// uart1 rx
	pio_pin_enable(PIO_B, 4, PIO_FUNC_D);	// uart1 tx

	return -r;
}
