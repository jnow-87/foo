/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_H
#define ARM_H


#include <config/config.h>


#ifdef CONFIG_ATSAMV71
#include <arch/arm/board/atsamv71.h>
#endif // CONFIG_ATSAMV71

#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#ifdef BUILD_KERNEL
#include <arch/arm/interrupt.h>
#endif // BUILD_KERNEL

#include <arch/arm/timebase.h>
#include <arch/arm/thread.h>
#include <arch/arm/atomic.h>
#include <arch/types.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* static variables */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

// kernel callbacks
#ifdef BUILD_KERNEL

static arch_callbacks_kernel_t const arch_cbs_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = 0x0,			/* TODO */
	.core_panic = 0x0,			/* TODO */

	/* virtual memory management */
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	/* interrupts */
	.int_enable = 0x0,			/* TODO */
	.int_enabled = 0x0,			/* TODO */

	.int_ipi = 0x0,

	/* threading */
	.thread_context_init = 0x0,	/* TODO */
	.thread_context_type = 0x0,	/* TODO */

	/* terminal I/O */
#ifdef CONFIG_AVR_UART
	.putchar = 0x0,				/* TODO */
	.puts = 0x0,				/* TODO */
#else
	.putchar = 0x0,
	.puts = 0x0,
#endif // CONFIG_KERNEL_UART
};

#endif // BUILD_KERNEL

// common callbacks
static arch_callbacks_common_t const arch_cbs_common = {
	.timebase = 0x0,			/* TODO */
	.timebase_to_time = 0x0,	/* TODO */

	/* atomics */
	.cas = arm_cas,

	/* syscall */
	.sc = 0x0,					/* TODO */

	/* main entry */
#ifdef BUILD_KERNEL
	.lib_crt0 = 0x0,
#else
	.lib_crt0 = 0x0,			/* TODO */
#endif // BUILD_KERNEL
};

// architecture info
static arch_info_t const arch_info = {
	.kernel_timer_err_us = 0,	/* TODO */
	.sched_timer_err_us = 0,	/* TODO */
};

#endif // _x86_
#endif // __x86_64__
#endif // ASM


#endif // ARM_H
