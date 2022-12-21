/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* types */
typedef void (*thread_modifier_t)(thread_t *this_t, void *payload);


/* prototypes */
void sched_yield(void);
void sched_trigger(void);

thread_t *sched_running(void);
thread_t *sched_running_nopanic(void);

void sched_thread_modify(thread_t *this_t, thread_modifier_t op, void *payload, size_t size);
void sched_thread_pause(thread_t *this_t);
void sched_thread_wake(thread_t *this_t);
void sched_thread_bury(thread_t *this_t);


#endif // KERNEL_SCHED_H
