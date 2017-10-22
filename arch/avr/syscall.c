#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/register.h>


/* macros */
#define PCMSK_BIT	(CONFIG_SC_PCINT % 8)


/* types */
typedef struct{
	sc_t num;
	void *param;
	size_t psize;
} avr_sc_arg_t;


/* global functions */
#ifdef BUILD_LIBSYS
void avr_sc(sc_t num, void *param, size_t psize){
	static volatile avr_sc_arg_t arg;


	/* prepare paramter */
	arg.num = num;
	arg.param = param;
	arg.psize = psize;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, lo8(&arg));
	mreg_w(GPIOR1, hi8(&arg));

	/* trigger syscall */
	asm volatile("sei");	// FIXME: from time to time interrupts are disabled
							//		  for no known reason
	mreg_w(CONFIG_SC_PIN, (0x1 << CONFIG_SC_PIN_BIT));
}
#endif // BUILD_LIBSYS

#ifdef BUILD_KERNEL
int avr_sc_hdlr(void){
	avr_sc_arg_t *arg;


	/* reset interrupt flag */
	mreg_w(PCIFR, (0x1 << CONFIG_SC_PCIFR_FLAG));

	/* acquire parameter */
	// get address from GPIO registers 0/1
	arg = (avr_sc_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	/* call kernel syscall handler */
	return ksc_hdlr(arg->num, arg->param, arg->psize);
}
#endif // BUILD_KERNEL


/* local functions */
#ifdef BUILD_KERNEL
static int init(void){
	/* enable interrupt used to trigger a syscall */
	// enable configured pin change interrupt
	mreg_w(PCICR, (0x1 << CONFIG_SC_PCICR_IE));
	mreg_w(CONFIG_SC_PCMSK, (0x1 << PCMSK_BIT));

	// set respective pin data direction to output
	mreg_w(CONFIG_SC_DDR, (0x1 << CONFIG_SC_PIN_BIT));

	return E_OK;
}

driver_init(init);
#endif // BUILD_KERNEL
