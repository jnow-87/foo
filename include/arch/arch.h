/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_H
#define ARCH_H


#ifndef ASM
# ifdef BUILD_KERNEL
#  include <kernel/interrupt.h>
#  include <kernel/memory.h>
# endif // BUILD_KERNEL

# include <sys/syscall.h>
# include <sys/thread.h>
# include <sys/types.h>
#endif // ASM


/* incomplete types */
#ifndef ASM
# ifdef BUILD_KERNEL
struct process_t;
struct thread_t;
struct thread_ctx_t;
# endif // BUILD_KERNEL
#endif //ASM


/* types */
#ifndef ASM
# ifdef BUILD_KERNEL
typedef struct{
	/* core */
	int (*core_id)(void);
	void (*core_sleep)(void);

	void (*core_panic)(struct thread_ctx_t const *tc);

	/* virtual memory management */
	int (*page_entry_write)(struct page_t const *page);
	int (*page_entry_inval_idx)(unsigned int idx, bool sync_cores);
	int (*page_entry_inval_va)(void *virt_addr, bool sync_cores);
	int (*page_entry_search)(struct page_t const *param, struct page_t *result);

	int (*copy_from_user)(void *kernel, void const *user, size_t n, struct process_t const *this_p);
	int (*copy_to_user)(void *user, void const *kernel, size_t n, struct process_t const *this_p);

	/* interrupts */
	int_type_t (*int_enable)(int_type_t mask);
	int_type_t (*int_enabled)(void);

	void (*int_ipi)(unsigned int core, bool bcast);

	/* threading */
	void (*thread_ctx_init)(struct thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);
} arch_ops_kernel_t;
# endif // BUILD_KERNEL

typedef struct{
	/* atomics */
	int (*cas)(int volatile *v, int old, int new);
	void (*atomic_inc)(int volatile *v, int inc);

	/* syscall */
	int (*sc)(sc_num_t num, void *param, size_t psize);
} arch_ops_common_t;

typedef struct{
	int kernel_timer_err_us,
		sched_timer_err_us;
} arch_info_t;
#endif // ASM


/* macros */
# ifdef BUILD_KERNEL
#  define arch_kernel_call(p, err_ret) \
	(arch_ops_kernel.p == 0x0) ? (err_ret) : arch_ops_kernel.p
# endif // BUILD_KERNEL

#define arch_common_call(p, err_ret) \
	(arch_ops_common.p == 0x0) ? (err_ret) : arch_ops_common.p

#define arch_info(c) \
	arch_info.c


/* include architecture header */
#include BUILD_ARCH_HEADER


#endif // ARCH_H
