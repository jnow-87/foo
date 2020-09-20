/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef BRICKOS_X86_H
#define BRICKOS_X86_H


#ifndef ASM

#include <arch/x86/thread.h>

#endif // ASM


/* static variables */
#ifndef ASM

// kernel callbacks
#ifdef BUILD_KERNEL

static arch_callbacks_kernel_t const arch_cbs_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = 0x0,	// TODO
	.core_panic = 0x0,	// TODO

	/* virtual memory management */
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	/* interrupts */
	.int_enable = 0x0,		// TODO
	.int_enabled = 0x0,		// TODO

	.int_ipi = 0x0,

	/* threading */
	.thread_context_init = 0x0,	// TODO
	.thread_context_type = 0x0,	// TODO
};

#endif // BUILD_KERNEL

// common callbacks
static arch_callbacks_common_t const arch_cbs_common = {
	/* atomics */
	.cas = 0x0,			// TODO
	.atomic_inc = 0x0,	// TODO

	/* syscall */
	.sc = 0x0,
};

// architecture info
static arch_info_t const arch_info = {
	.kernel_timer_err_us = 0,	// TODO
	.sched_timer_err_us = 0,	// TODO
};

#endif // ASM


#endif // BRICKOS_X86_H
