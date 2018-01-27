#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* external variables */
extern process_t *process_table;


/* prototypes */
void sched_tick(void);

thread_t const *sched_running(void);

void sched_yield(void);
void sched_pause(void);
void sched_wake(thread_t const *this_t);


#endif // KERNEL_SCHED_H
