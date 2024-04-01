/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef BRICKOS_X86_H
#define BRICKOS_X86_H


#include <arch/types.h>
#include <arch/x86/hardware.h>
#include <sys/syscall.h>
#include <sys/thread.h>
#include <sys/types.h>


/* incomplete types */
#ifdef BUILD_KERNEL
struct process_t;
struct thread_t;
#endif // BUILD_KERNEL


/* types */
#ifdef BUILD_KERNEL
typedef struct thread_ctx_t{
	struct thread_ctx_t *next,
						*this;

	int type;	/**< cf. thread_ctx_type_t */
	thread_entry_t entry;
	void *arg;

	sc_t sc_arg;
	x86_hw_op_t sc_op;
} thread_ctx_t;
#endif // BUILD_KERNEL

typedef enum{
	OLOC_NONE = 0x0,
	OLOC_PRE = 0x1,
	OLOC_POST = 0x2,
} x86_sc_overlay_loc_t;

typedef struct{
	int (*call)(void *);
	x86_sc_overlay_loc_t loc;
} x86_sc_overlay_t;


/* prototypes */
#ifdef BUILD_KERNEL
// core
void x86_core_sleep(void);
void x86_core_panic(thread_ctx_t const *tc);

// memory
int x86_copy_from_user(void *kernel, void const *user, size_t n, struct process_t const *this_p);
int x86_copy_to_user(void *user, void const *kernel, size_t n, struct process_t const *this_p);

// interrupt
bool x86_int_enable(bool en);
bool x86_int_enabled(void);
x86_hw_op_t *x86_int_op(void);

// thread
void x86_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);

// kernel syscall
sc_t *x86_sc_arg(struct thread_t *this_t);
void x86_sc_epilogue(struct thread_t *this_t);

// scheduler
void x86_sched_trigger(void);
void x86_sched_yield(void);
void x86_sched_wait(struct thread_t *this_t);
void x86_sched_force(struct thread_t *this_t);

// init and cleanup
void x86_std_init(void);
void x86_std_fini(void);

void x86_memory_cleanup(void);
int x86_rootfs_dump(void);
#endif // BUILD_KERNEL

// atomics
int x86_cas(int volatile *v, int old, int new);
void x86_atomic_add(int volatile *v, int inc);

// common syscall
int x86_sc(sc_num_t num, void *param, size_t psize);
int x86_sc_overlay_call(sc_num_t num, void *param, x86_sc_overlay_loc_t loc, x86_sc_overlay_t *overlays);


/* static variables */
#ifdef BUILD_KERNEL
// kernel ops
static arch_ops_kernel_t const arch_ops_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = x86_core_sleep,
	.core_panic = x86_core_panic,
	.cores_boot = 0x0,

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

	.ipi_int = 0x0,
	.ipi_arg = 0x0,

	/* threading */
	.thread_ctx_init = x86_thread_ctx_init,

	/* syscall */
	.sc_arg = x86_sc_arg,
};
#endif // BUILD_KERNEL

// common ops
static arch_ops_common_t const arch_ops_common = {
	/* atomics */
	.atomic = 0x0,
	.cas = x86_cas,
	.atomic_add = x86_atomic_add,

	/* syscall */
	.sc = x86_sc,
};


#endif // BRICKOS_X86_H
