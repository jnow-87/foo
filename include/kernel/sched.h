#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* external variables */
extern process_t *process_table;


/* prototypes */
void sched_resched(void);


#endif // KERNEL_SCHED_H
