#ifndef AVR_THREAD_H
#define AVR_THREAD_H


#include <sys/types.h>


/* types */
typedef struct thread_context_t{
	uint8_t	sreg,
			mcusr,
			rampz;

	uint8_t gpr[32];
} thread_context_t;


#endif // AVR_THREAD_H
