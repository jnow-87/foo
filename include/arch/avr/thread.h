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
enum thread_ctx_type_t;


/* types */
// NOTE when changing thread_ctx_t also check if modifications
// 		to the interrupt service routine are required
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;

	uint8_t errno;			/**< kernel errno */

	uint8_t sreg,			/**< status register */
			mcusr,			/**< control register */
			rampz;			/**< extended Z-pointer */

	uint8_t gpior[3];		/**< GPIO registers */
	uint8_t gpr[32];		/**< general purpose registers */

	void *int_vec_addr,		/**< int_vectors[cur + 1] */
		 *ret_addr;			/**< thread return address on interrupt */
} thread_ctx_t;


/* prototypes */
void avr_thread_context_init(thread_ctx_t *ctx, struct thread_t *this_t, user_entry_t user_entry, thread_entry_t thread_entry, void *thread_arg);
enum thread_ctx_type_t avr_thread_context_type(thread_ctx_t *ctx);


#endif // AVR_THREAD_H
