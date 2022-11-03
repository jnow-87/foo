/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/compiler.h>
#include <fcntl.h>


/* local/static prototypes */
static void lnx_verify_constants(void) __unused;


/* local functions */
static void lnx_verify_constants(void){
	STATIC_ASSERT(LNX_O_RDONLY == O_RDONLY);
	STATIC_ASSERT(LNX_O_WRONLY == O_WRONLY);
	STATIC_ASSERT(LNX_O_RDWR == O_RDWR);
	STATIC_ASSERT(LNX_O_CREAT == O_CREAT);
}
