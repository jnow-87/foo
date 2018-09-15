#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* macros */
#ifdef CONFIG_SCHED_PREEMPTIVE
#define sched_tick()	sched_trigger()
#else
#define sched_tick()
#endif // CONFIG_SCHED_PREEMPTIVE



/* prototypes */
void sched_yield(void);
void sched_trigger(void);
thread_t const *sched_running(void);

void sched_thread_pause(thread_t *this_t);
void sched_thread_wake(thread_t *this_t);
void sched_thread_bury(thread_t *this_t);


#endif // KERNEL_SCHED_H
