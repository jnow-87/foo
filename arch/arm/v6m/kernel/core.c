/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/v6m.h>
#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/string.h>


/* macros */
// exception priorities
// NOTE systick gets the second highest priority to increase the kernel timer accuracy by preventing longer running
// 		syscalls from delaying timer ticks.
#define SYSTICK_PRIO	1

// NOTE pendsv gets a higher priority than systick to prevent scheduler transitions while a pendsv exception is pending.
// 		Since pendsv, unlike svcall, is asynchronous to program execution, it is possible for another exception to occur
// 		between setting pendsv pending and actually taking its exception. If that other exception would be a systick and
// 		systick would have a higher priority then pendsv, the systick would be taken first, potentially causing a
// 		scheduler transition. In that case the pendsv exception would still be pending, hence, instead of returning to
// 		the newly scheduled thread, the pendsv exception would be taken but with the thread that requested the pendsv no
// 		longer being active, hence using an invalid context and a thus invalid syscall parameter pointer.
//
// 		pendsv also has to have higher priority than svcall, since pendsv is used to trigger syscalls from within the
// 		kernel. If the pendsv priority would not be higher than the svcall priority pendsv exceptions would be handled
// 		after svcall, which would, for instance, render the kernel signal implementation invalid, since it relies on a
// 		context switch within a syscall.
#define PENDSV_PRIO		0
#define SVCALL_PRIO		2

// NOTE Priorities of external interrupts need to be lower than for core exceptions, especially svcall, to allow svc
// being issued from within an interrupt without causing a hardfault.
#define EXTERN_PRIO		3


/* global functions */
int av6m_core_init(uint32_t clock_mhz){
	ppb_write(SCR, ((0x0 << SCR_SEVONPEND) | (0x1 << SCR_SLEEPDEEP) | (0x0 << SCR_SLEEPONEXIT)));

	ppb_write(SHPR2, (SVCALL_PRIO << SHPR2_PRI11));
	ppb_write(SHPR3, ((SYSTICK_PRIO << SHPR3_PRI15) | (PENDSV_PRIO << SHPR3_PRI14)));

	av6m_nvic_init();

	for(int i=0; i<32; i++)
		av6m_nvic_int_prio_set(i, EXTERN_PRIO);

	return av6m_systick_init(clock_mhz);
}

void av6m_core_sleep(void){
	asm volatile("wfe");
}

void av6m_core_panic(thread_ctx_t const *tc){
	register_t gpr[13];


	/* dump registers */
	if(tc != 0x0){
		kprintf(KMSG_ANY, "config and status registers\n"
			"%20.20s: %#8.8x\n"
			"%20.20s: %#8.8x\n"
			"%20.20s: %#8.8x\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %8.8p\n"
			"%20.20s: %u\n\n"
			,
			"CONTROL", mrs(control),
			"PRIMASK", mrs(primask),
			"xPSR", tc->xpsr,
			"MSP", mrs(msp),
			"PSP", mrs(psp),
			"SP", tc,
			"VTOR", ppb_read(VTOR),
			"EXC_RETURN", tc->exc_return,
			"LR(ret)", tc->ret_lr,
			"interrupted at", tc->ret_addr,
			"interrupt vector", ppb_read(ICSR) & 0x1ff
		);

		kprintf(KMSG_ANY, "general purpose registers\n");

		memcpy(gpr + 0, tc->gpr_0_3, sizeof(register_t) * 4);
		memcpy(gpr + 4, tc->gpr_4_7, sizeof(register_t) * 4);
		memcpy(gpr + 8, tc->gpr_8_11, sizeof(register_t) * 4);
		gpr[12] = tc->gpr_12;

		for(uint8_t i=0; i<13; i++){
			kprintf(KMSG_ANY, "\t%2.2u: %#8.8x", i, gpr[i]);

			if(i % 4 == 3)
				kprintf(KMSG_ANY, "\n");
		}

		kprintf(KMSG_ANY, "\n");
	}
	else
		kprintf(KMSG_ANY, "unknown thread context\n");
}
