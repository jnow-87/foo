/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_THREAD_H
#define AVR_THREAD_H


#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/devicetree.h>
#include <sys/thread.h>


/* incomplete types */
struct thread_t;


/* types */
// NOTE when changing thread_ctx_t also check if modifications
// 		to the interrupt service routine are required
// NOTE size assumptions on certain fields, e.g. type, are checked
// 		in the interrupt handler
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;

	struct thread_ctx_t *this;
	uint8_t type;				/**< cf. thread_ctx_type_t */

	uint8_t sreg,				/**< status register */
			mcusr,				/**< control register */
			rampz;				/**< extended Z-pointer */

	uint8_t gpior[3];			/**< GPIO registers */
	uint8_t gpr[32];			/**< general purpose registers */

	void *int_vec_addr,			/**< level-1 interrupt vector return address */
		 *ret_addr;				/**< thread return address on interrupt */
} thread_ctx_t;

STATIC_ASSERT(sizeof(((thread_ctx_t*)(0))->next) == (DEVTREE_ARCH_ADDR_WIDTH == 16 ? 2 : 1));
STATIC_ASSERT(sizeof(((thread_ctx_t*)(0))->type) == 1);


/* prototypes */
void avr_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);


#endif // AVR_THREAD_H
