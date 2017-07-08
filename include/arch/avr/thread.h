#ifndef AVR_THREAD_H
#define AVR_THREAD_H


#include <sys/types.h>


/* incomplete types */
struct thread_t;


/* types */
typedef struct thread_context_t{
	uint8_t	sreg,
			mcusr,
			rampz;

	uint8_t gpr[32];
	void *ret_addr;
} thread_context_t;

typedef uint8_t thread_id_t;


/* prototypes */
thread_context_t *avr_thread_context_init(struct thread_t *this_t, void *thread_arg);


#endif // AVR_THREAD_H
