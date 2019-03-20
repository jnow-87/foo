/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/board/atmel/pmc.h>


/* global functions */
int atsamv71_init(void){
	int r;


	r = 0;
	r |= pmc_init();

	return -r;
}
