#include <arch/arch.h>
#include <kernel/init.h>
#include <sys/error.h>


/* global functions */
void avr_core_sleep(void){
	/* set sleep mode */
	mreg_w(SMCR, (0x1 << SMCR_SE) |
#if CONFIG_AVR_SLEEPMODE_IDLE
		(0x0 << SMCR_SM)
#elif CONFIG_AVR_SLEEPMODE_ADCNR
		(0x1 << SMCR_SM)
#elif CONFIG_AVR_SLEEPMODE_PWRDWN
		(0x2 << SMCR_SM)
#elif CONFIG_AVR_SLEEPMODE_PWRSAVE
		(0x3 << SMCR_SM)
#elif CONFIG_AVR_SLEEPMODE_STANDBY
		(0x6 << SMCR_SM)
#elif CONFIG_AVR_SLEEPMODE_EXTSTANDBY
		(0x7 << SMCR_SM)
#else
	#error "invalid sleep mode, check kernel config"
#endif // CONFIG_AVR_SLEEPMODE
	);

	/* send core to sleep */
	asm volatile("sleep");

	/* prevent accidental sleep */
	mreg_w(SMCR, 0x0);
}

void avr_core_halt(void){
	asm volatile(
		"break\n"
		"sleep\n"
	);
}


/* local functions */
static error_t avr_core_init(void){
	/* set MCUCR[IVSEL], moving interrupt vectors to boot flash */
	mreg_bit_set_sync(MCUCR, MCUCR_IVSEL, MCUCR_IVCE);

	return E_OK;
}

core_init(0, avr_core_init);
