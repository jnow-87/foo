/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_CALLBACKS_H
#define ARCH_CALLBACKS_H


#ifndef ASM
#ifdef BUILD_KERNEL

#include <arch/interrupt.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/memory.h>

#endif // BUILD_KERNEL

#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/thread.h>
#include <sys/time.h>
#include <sys/types.h>

#endif // ASM


/* incomplete types */
#ifdef BUILD_KERNEL

struct process_t;
struct thread_t;
enum thread_ctx_type_t;

#endif // BUILD_KERNEL


/* types */
#ifdef BUILD_KERNEL

typedef struct{
	/* core */
	int (*core_id)(void);
	void (*core_sleep)(void);

	void (*core_panic)(thread_ctx_t const *tc);

	/* virtual memory management */
	int (*page_entry_write)(page_t const *page);
	int (*page_entry_inval_idx)(unsigned int idx, bool sync_cores);
	int (*page_entry_inval_va)(void *virt_addr, bool sync_cores);
	int (*page_entry_search)(page_t const *param, page_t *result);

	void (*copy_from_user)(void *target, void const *src, unsigned int n, struct process_t const *this_p);
	void (*copy_to_user)(void *target, void const *src, unsigned int n, struct process_t const *this_p);

	/* interrupts */
	int (*int_register)(int_num_t num, int_hdlr_t hdlr, void *data);
	void (*int_release)(int_num_t num);

	void (*int_call)(int_num_t num);

	int_type_t (*int_enable)(int_type_t mask);
	int_type_t (*int_enabled)(void);

	void (*int_ipi)(unsigned int core, bool bcast);

	/* threading */
	void (*thread_context_init)(thread_ctx_t *ctx, struct thread_t *this_t, user_entry_t user_entry, thread_entry_t thread_entry, void *thread_arg);
	enum thread_ctx_type_t (*thread_context_type)(thread_ctx_t *ctx);
} arch_callbacks_kernel_t;

#endif // BUILD_KERNEL

typedef struct{
	/* time */
	timebase_t * (*timebase)(void);
	time_t * (*timebase_to_time)(timebase_t *tb);

	/* atomics */
	int (*cas)(volatile int *v, int old, int new);

	/* syscall */
	int (*sc)(sc_t num, void *param, size_t psize);

	/* libsys functionality */
	int (*lib_crt0)(void);
} arch_callbacks_common_t;

typedef struct{
	int kernel_timer_err_us,
		sched_timer_err_us;
} arch_info_t;


#endif
