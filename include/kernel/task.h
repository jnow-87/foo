#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H


#include <sys/types.h>


/* incomplete types */
struct ktask_queue_t;


/* types */
typedef void (*ktask_hdlr_t)(void *p);

typedef struct ktask_t{
	struct ktask_t *prev,
				   *next;

	struct ktask_queue_t *queue;
	struct ktask_t *queue_next;

	bool ready;

	ktask_hdlr_t hdlr;
	char data[];
} ktask_t;

typedef struct ktask_queue_t{
	ktask_t * volatile head,
			* volatile tail;

	struct ktask_queue_t *prev,
						 *next;
} ktask_queue_t;


/* prototypes */
int ktask_create(ktask_hdlr_t hdlr, void *data, size_t size, ktask_queue_t *queue);
void ktask_complete(ktask_t *task);
ktask_t *ktask_next(void);

ktask_queue_t *ktask_queue_create(void);
void ktask_queue_destroy(ktask_queue_t *queue);
void ktask_queue_flush(ktask_queue_t *queue);


#endif // KERNEL_TASK_H
