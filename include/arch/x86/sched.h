/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_SCHED_H
#define X86_SCHED_H


#include <kernel/thread.h>


/* prototypes */
void x86_sched_trigger(void);
void x86_sched_yield(void);
void x86_sched_wait(thread_t const *this_t);
void x86_sched_force(thread_t const *this_t);


#endif // X86_SCHED_H
