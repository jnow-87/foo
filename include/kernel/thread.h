/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H


#include <arch/thread.h>
#include <kernel/process.h>
#include <sys/thread.h>
#include <sys/errno.h>


/* incomplete types */
struct process_t;
struct page_t;


/* types */
typedef struct thread_t{
	struct thread_t *prev,
					*next;

	tid_t tid;

	unsigned int affinity,
				 priority;

	void *entry;
	struct page_t *stack;
	thread_state_t state;
	thread_context_t *ctx;

	struct process_t *parent;
} thread_t;


/* prototypes */
thread_t *thread_create(struct process_t *this_p, tid_t tid, void *entry, void *thread_arg);
void thread_destroy(struct thread_t *this_t);


#endif // KERNEL_THREAD_H
