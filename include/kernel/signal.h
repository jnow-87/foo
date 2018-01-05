#ifndef KERNEL_SIGNAL_H
#define KERNEL_SIGNAL_H


#include <kernel/thread.h>


/* types */
typedef struct ksignal_el_t{
	struct thread_t const *thread;

	struct ksignal_el_t *prev,
					   *next;
} ksignal_el_t;

typedef ksignal_el_t * ksignal_t;


/* prototypes */
void ksignal_init(ksignal_t *sig);

int ksignal_wait(ksignal_t *sig);
int ksignal_send(ksignal_t *sig);
int ksignal_bcast(ksignal_t *sig);


#endif // KERNEL_SIGNAL_H
