#ifndef LOC_KERNEL_SCHED_H
#define LOC_KERNEL_SCHED_H


#include <kernel/csection.h>


/* external variables */
extern csection_lock_t sched_lock;


/* prototypes */
void sched_transition(thread_t *this_t, thread_state_t queue);


#endif // LOC_KERNEL_SCHED_H
