#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H


#include <sys/types.h>


/* types */
typedef void (*ktask_hdlr_t)(void *p);

typedef struct ktask_t{
	struct ktask_t *prev,
				   *next;

	ktask_hdlr_t hdlr;
	char data[];
} ktask_t;


/* prototypes */
int ktask_create(ktask_hdlr_t hdlr, void *data, size_t size);
void ktask_destroy(ktask_t *task);

ktask_t *ktask_next(void);


#endif // KERNEL_TASK_H
