#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
#define PCMSK_BIT	(CONFIG_AVR_SYSCALL_PCINT % 8)


/* types */
typedef struct{
	syscall_t num;
	void *param;
	size_t psize;
} avr_syscall_arg_t;


/* local/static prototypes */
#ifdef KERNEL
static int avr_syscall_hdlr(int_num_t num);
#endif // KERNEL


/* global functions */
#ifdef LIBSYS
avr_syscall_arg_t syscall_arg;

int avr_syscall(syscall_t num, void *param, size_t psize){
	static volatile avr_syscall_arg_t arg;


	/* prepare paramter */
	arg.num = num;
	arg.param = param;
	arg.psize = psize;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, (uint8_t)(((unsigned int)(&arg)) & 0xff));
	mreg_w(GPIOR1, (uint8_t)(((unsigned int)(&arg)) >> 8));

	/* trigger syscall */
	mreg_w(CONFIG_AVR_SYSCALL_PIN, (0x1 << CONFIG_AVR_SYSCALL_PIN_BIT));

	return E_OK;
}
#endif // LIBSYS


/* local functions */
#ifdef KERNEL
static int avr_syscall_init(void){
	/* enable interrupt used to trigger a syscall */
	// enable configured pin change interrupt
	mreg_w(PCICR, (0x1 << CONFIG_AVR_SYSCALL_PCICR_IE));
	mreg_w(CONFIG_AVR_SYSCALL_PCMSK, (0x1 << PCMSK_BIT));

	// set respective pin data direction to output
	mreg_w(CONFIG_AVR_SYSCALL_DDR, (0x1 << CONFIG_AVR_SYSCALL_PIN_BIT));

	/* register interrupt handler */
	int_hdlr_register(CONFIG_AVR_SYSCALL_INT, avr_syscall_hdlr);

	return E_OK;
}

driver_init(avr_syscall_init);
#endif // KERNEL

#ifdef KERNEL
static int avr_syscall_hdlr(int_num_t num){
	avr_syscall_arg_t *arg;


	/* reset interrupt flag */
	mreg_w(PCIFR, (0x1 << CONFIG_AVR_SYSCALL_PCIFR_FLAG));

	/* acquire parameter */
	// get address from GPIO registers 0/1
	arg = (avr_syscall_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	/* call kernel syscall handler */
	return ksyscall_hdlr(arg->num, arg->param, arg->psize);
}
#endif // KERNEL
