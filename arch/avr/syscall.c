#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/syscall.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/register.h>
#include <sys/compiler.h>


/* macros */
#define INT_VEC_SC		(CONFIG_KERNEL_TEXT_BASE + INT_VEC_SIZE * INT_VECTORS)
#define SYSCALL(addr)	asm volatile("call " STRGIFY(addr));


/* types */
typedef struct{
	sc_t num;
	void *param;
	size_t psize;
} avr_sc_arg_t;


/* global functions */
void avr_sc(sc_t num, void *param, size_t psize){
	volatile avr_sc_arg_t arg;


	/* clear interrupts */
	asm volatile("cli");

	/* prepare paramter */
	arg.num = num;
	arg.param = param;
	arg.psize = psize;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, lo8(&arg));
	mreg_w(GPIOR1, hi8(&arg));

	/* trigger syscall */
	SYSCALL(INT_VEC_SC);
}

#ifdef BUILD_KERNEL
void avr_sc_hdlr(void){
	avr_sc_arg_t *arg;


	/* acquire parameter */
	// get address from GPIO registers 0/1
	arg = (avr_sc_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	/* call kernel syscall handler */
	ksc_hdlr(arg->num, arg->param, arg->psize);
}
#endif // BUILD_KERNEL
