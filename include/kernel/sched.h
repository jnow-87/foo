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


/* prototypes */
void sched_yield(void);
void sched_trigger(void);

thread_t *sched_running(void);
thread_t *sched_running_nopanic(void);

void sched_thread_transition(thread_t *this_t, thread_state_t state);


#endif // KERNEL_SCHED_H
