/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/x86/linux.h>
#include <arch/x86/opts.h>
#include <kernel/init.h>


/* local/static prototypes */
static void cont_hdlr(int sig);


/* global functions */
void x86_core_sleep(void){
	lnx_pause();
}

void x86_core_panic(thread_ctx_t const *tc){
	lnx_exit(-1);
}


/* local functions */
static int init(void){
	if(!x86_opts.interactive)
		return 0;

	lnx_sigaction(CONFIG_X86EMU_CONT_SIG, cont_hdlr, 0x0);

	LNX_DEBUG("waiting for start signal\n");
	lnx_pause();
	LNX_DEBUG("starting kernel\n");

	return 0;
}

platform_init(0, first, init);

static void cont_hdlr(int sig){
	LNX_DEBUG("recv continue signal\n");
}
