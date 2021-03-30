/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_THREAD_H
#define X86_THREAD_H


#include <sys/thread.h>


/* incomplete types */
struct thread_t;


/* types */
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;

	struct thread_ctx_t *this;

	int type;	/**< cf. thread_ctx_type_t */
} thread_ctx_t;


/* prototypes */
void x86_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);


#endif // X86_THREAD_H
