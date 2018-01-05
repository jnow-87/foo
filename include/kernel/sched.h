#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* external variables */
extern process_t *process_table;


/* prototypes */
int sched_enqueue(thread_t *this_t, thread_state_t queue);
thread_t const *sched_running(void);

void sched_tick(void);
void sched_yield(thread_state_t target);


#endif // KERNEL_SCHED_H
