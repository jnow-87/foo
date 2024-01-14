/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H


#include <arch/arch.h>
#include <kernel/process.h>
#include <kernel/usignal.h>
#include <sys/mutex.h>
#include <sys/thread.h>
#include <sys/signal.h>
#include <sys/errno.h>


/* incomplete types */
struct process_t;
struct thread_t;
struct page_t;


/* types */
typedef void (*thread_dtor_hdlr_t)(struct thread_t *this_t, void *payload);

typedef enum thread_ctx_type_t{
	CTX_UNKNOWN = 0,
	CTX_KERNEL,
	CTX_USER,
	CTX_SIGRETURN,
} thread_ctx_type_t;

typedef struct thread_dtor_t{
	struct thread_dtor_t *prev,
						 *next;

	thread_dtor_hdlr_t hdlr;
	void *payload;
} thread_dtor_t;

typedef struct thread_t{
	struct thread_t *prev,
					*next;

	tid_t tid;
	thread_state_t state;

	unsigned int affinity,
				 priority;

	thread_entry_t entry;

	struct page_t *stack;
	thread_ctx_t *ctx_stack;

	usignal_t *signals;
	thread_dtor_t *dtors;

	struct process_t *parent;

	mutex_t mtx;
} thread_t;


/* prototypes */
thread_t *thread_create(struct process_t *this_p, tid_t tid, thread_entry_t entry, void *thread_arg);
void thread_destroy(struct thread_t *this_t);

void thread_ctx_push(thread_ctx_t *ctx);
thread_ctx_t *thread_ctx_pop(void);

int thread_dtor_register(thread_t *this_t, thread_dtor_hdlr_t hdlr, void *payload);
void thread_dtor_release(thread_t *this_t, thread_dtor_hdlr_t hdlr, void *payload);


#endif // KERNEL_THREAD_H
