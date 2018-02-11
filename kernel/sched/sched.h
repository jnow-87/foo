#ifndef LOC_KERNEL_SCHED_H
#define LOC_KERNEL_SCHED_H


#include <sys/mutex.h>


/* types */
typedef struct sched_queue_t{
	thread_t *thread;

	struct sched_queue_t *prev,
						 *next;
} sched_queue_t;


/* external variables */
extern mutex_t sched_mtx;
extern sched_queue_t *sched_queues[NTHREADSTATES];


/* prototypes */
void sched_transition(thread_t *this_t, thread_state_t queue);


#endif // LOC_KERNEL_SCHED_H
