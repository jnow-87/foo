/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/avr/register.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/register.h>
#include <sys/syscall.h>
#include <sys/devicetree.h>


/* macros */
#define INT_VEC_SC		(DEVTREE_KERNEL_FLASH_BASE + INT_VEC_SIZE * INT_SYSCALL)

#ifdef CONFIG_AVR_ISA_AVR4
# define SYSCALL(addr)	\
	asm volatile( \
		"ldi	r30, lo8(" STRGIFY(addr) ")" \
		"ldi	r31, hi8(" STRGIFY(addr) ")" \
		"icall" \
	);
#else
# define SYSCALL(addr)	asm volatile("call " STRGIFY(addr));
#endif // CONFIG_AVR_ISA_AVR4


/* global functions */
int avr_sc(sc_num_t num, void *param, size_t psize){
	sc_t volatile sc;


	/* clear interrupts */
	asm volatile("cli");

	/* prepare paramter */
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errno = E_UNKNOWN;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, lo8(&sc));
	mreg_w(GPIOR1, hi8(&sc));

	/* trigger syscall */
	SYSCALL(INT_VEC_SC);

	if(sc.errno)
		return_errno(sc.errno);

	return 0;
}
