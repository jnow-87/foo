#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/register.h>


/* external prototypes */
extern void __isr_reset(void);


/* global variables */
uint8_t inkernel_nest = 0;


/* local/static prototypes */
static int avr_warm_reset_hdlr(int_num_t num);


/* static variables */
static int_hdlr_t int_map[NINTERRUPTS] = { 0x0 };


/* global functions */
struct thread_context_t *avr_int_hdlr(struct thread_context_t *tc){
	int terrno;
	int_num_t num;


	/* save thread context if not interrupting the kernel */
	if(inkernel_nest == 1)
		current_thread[PIR]->ctx = tc;

	/* call respective interrupt handler */
	// compute interrupt number
	num = ((void (*)(void))(lo8(tc->int_vec) | hi8(tc->int_vec)) - __isr_reset - INT_VEC_WORDS) / INT_VEC_WORDS;

	// check interrupt number
	if(num >= NINTERRUPTS)
		kernel_panic("out of bound interrupt num %d\n", num);

	// check handler
	if(int_map[num] == 0x0)
		kernel_panic("unhandled interrupt %d", num);

	// call handler, saving errno
	terrno = errno;
	errno = E_OK;

	if(int_map[num](num) != E_OK)
		WARN("error handling interrupt %u: %#x\n", num, errno);

	errno = terrno;

	/* return context of active thread */
	return current_thread[PIR]->ctx;
}

int avr_int_enable(int_type_t mask){
	if(mask)	asm volatile("sei");
	else		asm volatile("cli");

	return E_OK;
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;
	return INT_NONE;
}

int avr_int_hdlr_register(int_num_t num, int_hdlr_t hdlr){
	if(int_map[num] != 0x0 && int_map[num] != hdlr){
		WARN("interrupt already registerd %u %#x\n", num, int_map[num]);
		return_errno(E_INUSE);
	}

	int_map[num] = hdlr;

	return E_OK;
}

int avr_int_hdlr_release(int_num_t num){
	int_map[num] = 0x0;
	return E_OK;
}


/* static functions */
static int init(void){
	int_map[0] = avr_warm_reset_hdlr;
	return E_OK;
}

platform_init(0, init);

static int avr_warm_reset_hdlr(int_num_t num){
	kernel_panic("execute reset handler without actual MCU reset");

	return E_OK;
}
