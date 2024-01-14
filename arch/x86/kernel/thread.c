/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/thread.h>
#include <sys/string.h>


/* global functions */
void x86_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg){
	memset(ctx, 0, sizeof(thread_ctx_t));

	ctx->type = CTX_UNKNOWN;
	ctx->entry = entry;
	ctx->arg = arg;
}
