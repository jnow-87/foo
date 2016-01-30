#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H


#include <kernel/page.h>
#include <kernel/process.h>
#include <sys/compiler.h>


/* incomplete types */
struct process_t;


/* types */
typedef enum __packed{
	READY = 1,
	WAITING,
	RUNNING,
	FINISHED
} thread_state_t;

typedef struct thread_t{
	unsigned int tid,
				 affinity,
				 priority;

	void* entry;
	page_t* stack;
	thread_state_t state;

	struct process_t* parent;

	struct thread_t *next,
					*prev;
} thread_t;


/* prototypes */
thread_t* thread_create(struct process_t* const this_p, void* const entry);
void thread_destroy(struct thread_t* const this_t);
int thread_call(struct thread_t* const this);


#endif // KERNEL_THREAD_H
