/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/thread.h>
#include <sys/syscall.h>
#include <sys/stack.h>


/* global functions */
sc_t *avr_sc_arg(thread_t *this_t){
	thread_ctx_t *ctx;


	ctx = stack_top(this_t->ctx_stack);

	return (sc_t*)(ctx->gpior[0] | (ctx->gpior[1] << 8));
}
