#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>


/* external prototypes */
extern void __isr_reset(void);


/* external variables */
extern thread_t *current_thread[CONFIG_NCORES];


/* global variables */
uint8_t inkernel_nest = 0;
int_hdlr_t int_map[NINTERRUPTS] = { 0x0 };


/* global functions */
struct thread_context_t *avr_int_hdlr(struct thread_context_t *tc){
	uint8_t t;
	int terrno;
	int_num_t num;


	/* save thread context when coming from a process */
	if(inkernel_nest == 1)
		current_thread[PIR]->ctx = tc;

	/* call respective interrupt handler */
	// swap bytes of interrupt vector address
	t = (unsigned int)(tc->int_vec) >> 8;
	tc->int_vec = (void*)((unsigned int)(tc->int_vec) << 8 | t);

	// compute interrupt number
	num = ((void (*)(void))tc->int_vec - __isr_reset - INT_VEC_WORDS) / INT_VEC_WORDS;

	// check interrupt number
	if(num >= NINTERRUPTS){
		FATAL("out of bound interrupt num %d\n", num);
		core_halt();
	}

	if(int_map[num] == 0x0){
		FATAL("unhandled interrupt %d\n", num)
		core_halt();
	}

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

	return_errno(E_OK);
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;
	return INT_NONE;
}

int avr_int_hdlr_register(int_num_t num, int_hdlr_t hdlr){
	if(int_map[num] != 0x0){
		WARN("interrupt already registerd %u %#x\n", num, int_map[num]);
		return_errno(E_INUSE);
	}

	int_map[num] = hdlr;

	return_errno(E_OK);
}

int avr_int_hdlr_release(int_num_t num){
	int_map[num] = 0x0;
	return_errno(E_OK);
}
