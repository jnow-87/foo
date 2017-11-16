#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H


#include <arch/thread.h>
#include <kernel/process.h>
#include <sys/errno.h>


/* macros */
#define THREAD_ID_MAX	((thread_id_t)(~0))


/* incomplete types */
struct process_t;
struct page_t;


/* types */
typedef enum{
	CREATED = 1,
	READY,
	WAITING,
	RUNNING,
	FINISHED
} thread_state_t;

typedef struct thread_t{
	thread_id_t tid;

	unsigned int affinity,
				 priority;

	void *entry;
	struct page_t *stack;
	thread_state_t state;
	thread_context_t *ctx_lst;

	struct process_t *parent;

	struct thread_t *prev,
					*next;
} thread_t;


/* prototypes */
thread_t *thread_create(struct process_t *this_p, thread_id_t tid, void *entry, void *thread_arg);
void thread_destroy(struct thread_t *this_t);
void thread_context_enqueue(thread_t *this_t, thread_context_t *ctx);
thread_context_t *thread_context_dequeue(thread_t *this_t);


#endif // KERNEL_THREAD_H
