#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <sys/errno.h>


/* global functions */
void avr_sched_hdlr(void){
	// TODO convert watchdog frequency to kernel scheduler frequency

	sched_tick();
}


/* local functions */
static int init(void){
	/* clear reset flags */
	mreg_w(MCUSR, 0x0);

	/* set prescale value and interrupt */
	// start timed sequence
	mreg_w(WDTCSR,
		(0x1 << WDTCSR_WDCE) |
		(0x1 << WDTCSR_WDE)
	);

	// set config
	mreg_w(WDTCSR,
		(0x1 << WDTCSR_WDIE) |

#if defined(CONFIG_WATCHDOG_PRESCALE_2k)
		0x0
#elif defined(CONFIG_WATCHDOG_PRESCALE_4k)
		(0x1 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_8k)
		(0x2 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_16k)
		(0x3 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_32k)
		(0x4 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_64k)
		(0x5 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_128k)
		(0x6 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_256k)
		(0x7 << WDTCSR_WDP0)
#elif defined(CONFIG_WATCHDOG_PRESCALE_512k)
		(0x1 << WDTCSR_WDP3)
#elif defined(CONFIG_WATCHDOG_PRESCALE_1M)
		(0x1 << WDTCSR_WDP3) |
		(0x1 << WDTCSR_WDP0)
#endif // CONFIG_WATCHDOG_PRESCALE
	);

	return E_OK;
}

platform_init(0, init);
