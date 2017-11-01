#include <arch/thread.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>
#include <sys/register.h>


/* global functions */
void avr_core_sleep(void){
	/* set sleep mode */
	mreg_w(SMCR, (0x1 << SMCR_SE) |
#if defined(CONFIG_SLEEPMODE_IDLE)
		(0x0 << SMCR_SM)
#elif defined(CONFIG_SLEEPMODE_ADCNR)
		(0x1 << SMCR_SM)
#elif defined(CONFIG_SLEEPMODE_PWRDWN)
		(0x2 << SMCR_SM)
#elif defined(CONFIG_SLEEPMODE_PWRSAVE)
		(0x3 << SMCR_SM)
#elif defined(CONFIG_SLEEPMODE_STANDBY)
		(0x6 << SMCR_SM)
#elif defined(CONFIG_SLEEPMODE_EXTSTANDBY)
		(0x7 << SMCR_SM)
#else
	#error "invalid sleep mode, check kernel config"
#endif // CONFIG_SLEEPMODE
	);

	/* send core to sleep */
	asm volatile("sleep");

	/* prevent accidental sleep */
	mreg_w(SMCR, 0x0);
}

#ifdef BUILD_KERNEL
void avr_core_panic(thread_context_t const *tc){
	unsigned int i,
				 int_vec,
				 ret_addr;


	/* dump registers */
	int_vec = (((lo8(tc->int_vec) << 8) | hi8(tc->int_vec)) - INT_VEC_WORDS) * 2;
	ret_addr = ((lo8(tc->ret_addr) << 8) | hi8(tc->ret_addr)) * 2;

	kprintf(KMSG_ANY, "config and status registers\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %p\n"
		 "%20.20s: %4.4p\n"
		 "%20.20s: %4.4p\n\n"
		 ,
		 "SREG", tc->sreg,
		 "MCUSR", tc->mcusr,
		 "SP", tc + 1,
		 "interrupt vector", int_vec,
		 "interrupted at", ret_addr
	);

	kprintf(KMSG_ANY, "general purpose registers\n");

	for(i=0; i<32; i++){
		kprintf(KMSG_ANY, "\t%2.2u: %#2.2x", i, tc->gpr[i]);

		if(i % 4 == 3)
			kprintf(KMSG_ANY, "\n");
	}

	/* set sleep mode to power down */
	mreg_w(SMCR, (0x1 << SMCR_SE) | (0x2 << SMCR_SM));

	/* signal debugger */
	asm volatile("break");

	/* send core to sleep */
	asm volatile("sleep");
}
#endif // BUILD_KERNEL


/* local functions */
#ifdef BUILD_KERNEL
static int init(void){
	/* set MCUCR[IVSEL], moving interrupt vectors to boot flash if required */
#if CONFIG_KERNEL_TEXT_BASE == 0
	mreg_bit_clr_sync(MCUCR, MCUCR_IVSEL, MCUCR_IVCE);
#else
	mreg_bit_set_sync(MCUCR, MCUCR_IVSEL, MCUCR_IVCE);
#endif // CONFIG_KERNEL_TEXT_BASE

	return E_OK;
}

core_init(0, init);
#endif // BUILD_KERNEL
