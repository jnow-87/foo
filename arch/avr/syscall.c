#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
#define PCMSK_BIT	(CONFIG_AVR_SC_PCINT % 8)


/* types */
typedef struct{
	sc_t num;
	void *param;
	size_t psize;
} avr_sc_arg_t;


/* local/static prototypes */
#ifdef KERNEL
static int avr_sc_hdlr(int_num_t num);
#endif // KERNEL


/* global functions */
#ifdef LIBSYS
avr_sc_arg_t sc_arg;

int avr_sc(sc_t num, void *param, size_t psize){
	static volatile avr_sc_arg_t arg;


	/* prepare paramter */
	arg.num = num;
	arg.param = param;
	arg.psize = psize;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, (uint8_t)(((unsigned int)(&arg)) & 0xff));
	mreg_w(GPIOR1, (uint8_t)(((unsigned int)(&arg)) >> 8));

	/* trigger syscall */
	mreg_w(CONFIG_AVR_SC_PIN, (0x1 << CONFIG_AVR_SC_PIN_BIT));

	return E_OK;
}
#endif // LIBSYS


/* local functions */
#ifdef KERNEL
static int avr_sc_init(void){
	/* enable interrupt used to trigger a syscall */
	// enable configured pin change interrupt
	mreg_w(PCICR, (0x1 << CONFIG_AVR_SC_PCICR_IE));
	mreg_w(CONFIG_AVR_SC_PCMSK, (0x1 << PCMSK_BIT));

	// set respective pin data direction to output
	mreg_w(CONFIG_AVR_SC_DDR, (0x1 << CONFIG_AVR_SC_PIN_BIT));

	/* register interrupt handler */
	int_hdlr_register(CONFIG_AVR_SC_INT, avr_sc_hdlr);

	return E_OK;
}

driver_init(avr_sc_init);
#endif // KERNEL

#ifdef KERNEL
static int avr_sc_hdlr(int_num_t num){
	avr_sc_arg_t *arg;


	/* reset interrupt flag */
	mreg_w(PCIFR, (0x1 << CONFIG_AVR_SC_PCIFR_FLAG));

	/* acquire parameter */
	// get address from GPIO registers 0/1
	arg = (avr_sc_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	/* call kernel syscall handler */
	return ksc_hdlr(arg->num, arg->param, arg->psize);
}
#endif // KERNEL
