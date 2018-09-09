#ifndef KERNEL_SIGNAL_H
#define KERNEL_SIGNAL_H


/* incomplete types */
struct kthread_t;


/* types */
typedef struct ksignal_queue_t{
	struct ksignal_queue_t *next;

	struct thread_t const *thread;
} ksignal_queue_t;

typedef struct{
	ksignal_queue_t *head,
					*tail;
} ksignal_t;


/* prototypes */
void ksignal_init(ksignal_t *sig);

int ksignal_wait(ksignal_t *sig);
void ksignal_send(ksignal_t *sig);
void ksignal_bcast(ksignal_t *sig);


#endif // KERNEL_SIGNAL_H
