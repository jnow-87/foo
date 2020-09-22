/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef BRICKOS_X86_H
#define BRICKOS_X86_H


#ifndef ASM

#include <arch/x86/atomic.h>
#include <arch/x86/syscall.h>
#include <arch/x86/core.h>
#include <arch/x86/memory.h>
#include <arch/x86/interrupt.h>
#include <arch/x86/thread.h>

#endif // ASM


/* static variables */
#ifndef ASM

// kernel callbacks
#ifdef BUILD_KERNEL

static arch_callbacks_kernel_t const arch_cbs_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = x86_core_sleep,
	.core_panic = x86_core_panic,

	/* virtual memory management */
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = x86_copy_from_user,
	.copy_to_user = x86_copy_to_user,

	/* interrupts */
	.int_enable = x86_int_enable,
	.int_enabled = x86_int_enabled,

	.int_ipi = 0x0,

	/* threading */
	.thread_context_init = x86_thread_context_init,
	.thread_context_type = x86_thread_context_type,
};

#endif // BUILD_KERNEL

// common callbacks
static arch_callbacks_common_t const arch_cbs_common = {
	/* atomics */
	.cas = x86_cas,
	.atomic_inc = x86_atomic_add,

	/* syscall */
	.sc = x86_sc,
};

// architecture info
static arch_info_t const arch_info = {
	.kernel_timer_err_us = 0,
	.sched_timer_err_us = 0,
};

#endif // ASM


#endif // BRICKOS_X86_H
