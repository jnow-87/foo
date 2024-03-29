/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef BRICKOS_X86_H
#define BRICKOS_X86_H


#ifndef ASM
# include <arch/x86/atomic.h>
# include <arch/x86/syscall.h>
# include <arch/x86/core.h>
# include <arch/x86/memory.h>
# include <arch/x86/interrupt.h>
# include <arch/x86/thread.h>
#endif // ASM


/* static variables */
#ifndef ASM
# ifdef BUILD_KERNEL
// kernel ops
static arch_ops_kernel_t const arch_ops_kernel = {
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
	.thread_ctx_init = x86_thread_ctx_init,
};
# endif // BUILD_KERNEL

// common ops
static arch_ops_common_t const arch_ops_common = {
	/* atomics */
	.cas = x86_cas,
	.atomic_inc = x86_atomic_add,

	/* syscall */
	.sc = x86_sc,
};
#endif // ASM


#endif // BRICKOS_X86_H
