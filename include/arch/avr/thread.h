#ifndef AVR_THREAD_H
#define AVR_THREAD_H


#include <sys/types.h>


/* incomplete types */
struct thread_t;


/* types */
// NOTE when changing thread_context_t also check if modifications
// 		to the interrupt service routine are required
typedef struct thread_context_t{
	struct thread_context_t *next;

	uint8_t	sreg,			/**< status register */
			mcusr,			/**< control register */
			rampz;			/**< extended Z-pointer */
	uint8_t gpr[32];		/**< general purpose registers */

	void *ret_addr;			/**< thread return address on interrupt */
} thread_context_t;


/* prototypes */
thread_context_t *avr_thread_context_init(struct thread_t *this_t, void *proc_entry, void *thread_arg);


#endif // AVR_THREAD_H
