#include <arch/arch.h>
#include <arch/arm/v6m.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/ipi.h>
#include <kernel/timer.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/devicetree.h>
#include <sys/syscall.h>


/* external variables */
extern void (*__kernel_start[])(void);
extern void (*__kernel_end[])(void);


/* local/static prototypes */
static void hfault_hdlr(int_num_t num, void *payload);
static thread_ctx_type_t ctx_type(thread_ctx_t *ctx);


/* global functions */
thread_ctx_t *av6m_int_hdlr(thread_ctx_t *ctx){
	uint16_t num;


	ctx->this = ctx;
	ctx->type = ctx_type(ctx);

	thread_ctx_push(ctx);

	num = ICSR & 0x1ff;

	if(num == INT_PENDSV)
		num = INT_SVCALL;

//	INFO("int entry: num=%u\n", num);
	int_khdlr(num);

	return thread_ctx_pop();
}

void av6m_inval_hdlr(void){
	uint16_t num;


	num = ICSR & 0x1ff;
	kpanic("invalid interrupt %u\n", num);
}


/* local functions */
static int init(void){
	return int_register(INT_HARDFAULT, hfault_hdlr, 0x0);
}

platform_init(1, first, init);

static void hfault_hdlr(int_num_t num, void *payload){
	kpanic("core %u hard fault\n", PIR);
}

static thread_ctx_type_t ctx_type(thread_ctx_t *ctx){
	if(ctx->ret_addr >= (void*)__kernel_start && ctx->ret_addr <= (void*)__kernel_end)
		return CTX_KERNEL;

	return CTX_USER;
}
