#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* external variables */
extern thread_t *current_thread[CONFIG_NCORES];
extern process_t *process_table;


/* prototypes */
void sched_tick(void);
void sched_resched(void);


#endif // KERNEL_SCHED_H
