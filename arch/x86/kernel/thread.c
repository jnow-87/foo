/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/thread.h>
#include <kernel/thread.h>
#include <sys/string.h>


/* global functions */
void x86_thread_context_init(thread_ctx_t *ctx, struct thread_t *this_t, user_entry_t user_entry, thread_entry_t thread_entry, void *thread_arg){
	memset(ctx, 0, sizeof(thread_ctx_t));

	ctx->type = CTX_UNKNOWN;
}

enum thread_ctx_type_t x86_thread_context_type(thread_ctx_t *ctx){
	return ctx->type;
}
