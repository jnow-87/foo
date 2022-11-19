/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <sys/errno.h>
#include <sys/syscall.h>


/* local/static prototypes */
static void sc_hdlr(int_num_t num, void *payload);


/* local functions */
static int init(void){
	return int_register(INT_SYSCALL, sc_hdlr, 0x0);
}

platform_init(0, init);

static void sc_hdlr(int_num_t num, void *payload){
	sc_t *sc;


	sc = (sc_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	sc_khdlr(sc->num, sc->param, sc->size);
	sc->errno = errno;
}
