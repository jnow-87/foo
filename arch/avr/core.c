/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/thread.h>
#include <arch/avr/register.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>
#include <sys/register.h>
#include <sys/compiler.h>


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

GCC_ERROR(invalid sleep mode - check kernel config)

#endif // CONFIG_SLEEPMODE
	);

	/* send core to sleep */
	asm volatile("sleep");

	/* prevent accidental sleep */
	mreg_w(SMCR, 0x0);
}

#ifdef BUILD_KERNEL
void avr_core_panic(thread_ctx_t const *tc){
#ifdef CONFIG_KERNEL_PRINTF
	uint8_t i,
			j;
	uint16_t ret_addr_hi,
			 ret_addr_lo,
			 int_vec_addr_hi,
			 int_vec_addr_lo;
#endif // CONFIG_KERNEL_PRINTF


	/* dump registers */
#ifdef CONFIG_KERNEL_PRINTF
	if(tc != 0x0){
		ret_addr_hi = (lo8(tc->ret_addr) * 2) >> 8;
		ret_addr_lo = ((lo8(tc->ret_addr) << 8) | hi8(tc->ret_addr)) * 2;

		int_vec_addr_hi = (lo8(tc->int_vec_addr) * 2) >> 8;
		int_vec_addr_lo = ((lo8(tc->int_vec_addr) << 8) | hi8(tc->int_vec_addr)) * 2;

		kprintf(KMSG_ANY, "config and status registers\n"
			 "%20.20s: %#2.2x\n"
			 "%20.20s: %#2.2x\n"
			 "%20.20s: %p\n"
			 "%20.20s: %#x%4.4x\n"
			 "%20.20s: %#x%4.4x\n\n"
			 ,
			 "SREG", tc->sreg,
			 "MCUSR", tc->mcusr,
			 "SP", tc + 1,
			 "interrupted at", ret_addr_hi, ret_addr_lo,
			 "interrupt vector", int_vec_addr_hi, int_vec_addr_lo
		);

		kprintf(KMSG_ANY, "general purpose registers\n");

		for(i=0; i<32; i++){
			j = i / 4 + (i % 4) * 8;
			kprintf(KMSG_ANY, "\t%2.2u: %#2.2x", j, tc->gpr[j]);

			if(i % 4 == 3)
				kprintf(KMSG_ANY, "\n");
		}
	}
	else
		kprintf(KMSG_ANY, "unknown thread context\n");
#endif // CONFIG_KERNEL_PRINTF

	/* halt core */
	// set sleep mode to power down
	mreg_w(SMCR, (0x1 << SMCR_SE) | (0x2 << SMCR_SM));

	// signal debugger
	asm volatile("break");

	// send core to sleep
	asm volatile("sleep");
}
#endif // BUILD_KERNEL


/* local functions */
#ifdef BUILD_KERNEL
static int init(void){
	/* set system clock prescaler */
	mreg_w_sync(CLKPR, CONFIG_SYSTEM_CLOCK_PRESCALER, CLKPR_CLKPCE);

	/* set MCUCR[IVSEL], moving interrupt vectors to boot flash if required */
#if DEVTREE_KERNEL_FLASH_BASE == 0x0
	mreg_w_sync(MCUCR, (mreg_r(MCUCR) & (0xff ^ (0x1 << MCUCR_IVSEL))), MCUCR_IVCE);
#else
	mreg_w_sync(MCUCR, (mreg_r(MCUCR) | (0x1 << MCUCR_IVSEL)), MCUCR_IVCE);
#endif // DEVTREE_KERNEL_FLASH_BASE

	/* enable power reduction */
	mreg_w(PRR0, 0xff);
	mreg_w(PRR1, 0xff);

	return E_OK;
}

core_init(init);
#endif // BUILD_KERNEL
