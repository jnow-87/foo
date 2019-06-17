/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/init.h>
#include <arch/arch.h>
#include <arch/arm/board.h>


/* local functions */
static int init(void){
	return board_init();
}

core_init(init);
