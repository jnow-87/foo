#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H


#include <kernel/page.h>
#include <kernel/process.h>


/* incomplete types */
struct process_t;
struct thread_context_t;


/* types */
typedef enum{
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
	struct thread_context_t* ctx;

	struct process_t* parent;

	struct thread_t *next,
					*prev;
} thread_t;


/* prototypes */
thread_t* thread_create(struct process_t* const this_p, void* const entry);
void thread_destroy(struct thread_t* const this_t);
int thread_call(struct thread_t* const this);


#endif // KERNEL_THREAD_H
