#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H


#include <kernel/process.h>
#include <kernel/thread.h>


/* external variables */
extern process_t *process_table;


/* prototypes */
thread_t const *sched_running(void);
void sched_ctx_enqueue(thread_context_t *ctx);
thread_context_t *sched_ctx_dequeue(void);

void sched_tick(void);
void sched_resched(void);


#endif // KERNEL_SCHED_H
