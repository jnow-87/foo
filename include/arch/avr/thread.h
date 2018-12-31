/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_THREAD_H
#define AVR_THREAD_H


#include <sys/types.h>
#include <sys/thread.h>


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
void avr_thread_context_init(thread_context_t *ctx, struct thread_t *this_t, user_entry_t user_entry, thread_entry_t thread_entry, void *thread_arg);


#endif // AVR_THREAD_H
