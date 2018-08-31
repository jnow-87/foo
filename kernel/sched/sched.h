#ifndef LOC_KERNEL_SCHED_H
#define LOC_KERNEL_SCHED_H


#include <kernel/csection.h>


/* types */
typedef struct sched_queue_t{
	struct sched_queue_t *prev,
						 *next;

	thread_t *thread;
} sched_queue_t;


/* external variables */
extern csection_lock_t sched_lock;
extern sched_queue_t *sched_queues[NTHREADSTATES];


/* prototypes */
void sched_transition(thread_t *this_t, thread_state_t queue);


#endif // LOC_KERNEL_SCHED_H
