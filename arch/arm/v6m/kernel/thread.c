/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/thread.h>
#include <sys/string.h>
#include <sys/thread.h>


/* macros */
// register bits
#define CONTROL_NPRIV	0
#define CONTROL_SPSEL	1
#define EPSR_T			24

// exception return
#define EXC_RET_MAGIC	0xfffffff0
#define EXC_RET_MODE	3
#define EXC_RET_STACK	0


/* types */
typedef enum{
	ERM_HANDLER = 0,
	ERM_THREAD = 1,
} exc_ret_mode_t;

typedef enum{
	ERS_MAIN = 1,
	ERS_PROCESS = 5,
} exc_ret_stack_t;


/* global functions */
void av6m_thread_ctx_init(thread_ctx_t *ctx, thread_t *this_t, thread_entry_t entry, void *arg){
	memset(ctx, 0x0, sizeof(thread_ctx_t));

	ctx->type = CTX_UNKNOWN;

	// interrupt return
	ctx->exc_return = EXC_RET_MAGIC | (ERM_THREAD << EXC_RET_MODE) | (ERS_MAIN << EXC_RET_STACK);
	ctx->control = (0x1 << CONTROL_NPRIV) | (0x0 << CONTROL_SPSEL /* SPSEL can only be set through exc_return */);
	ctx->xpsr = (0x1 << EPSR_T);

	// thread entry
	// the first bit of the return address must not be set, since the
	// behaviour is undefined otherwise
	ctx->ret_addr = (void*)(CONFIG_INIT_BINARY & 0xfffffffe);

	// _start arguments
	ctx->gpr_0_3[0] = (register_t)entry;
	ctx->gpr_0_3[1] = (register_t)arg;
}
