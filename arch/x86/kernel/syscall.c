/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/compiler.h>
#include <sys/devtree.h>
#include <sys/types.h>
#include <sys/stack.h>
#include <sys/syscall.h>


/* local/static prototypes */
static void yield_user(thread_t *this_t);
static void yield_kernel(void);

static int overlay_exit(void *param);
static int overlay_mmap(void *param);


/* static variables */
// syscall overlays
static x86_sc_overlay_t overlays[] = {
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = overlay_mmap,	.loc = OLOC_POST },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = overlay_exit,	.loc = OLOC_PRE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
	{ .call = 0x0,			.loc = OLOC_NONE },
};

STATIC_ASSERT(sizeof_array(overlays) == NSYSCALLS);


/* global functions */
int x86_sc(sc_num_t num, void *param, size_t psize){
	thread_t *this_t;


	if(num != SC_SCHEDYIELD)
		LNX_EEXIT("kernel assumed to only use SC_SCHEDYIELD\n");

	this_t = sched_running();

	// Kernel and user thread yield calls need to be differentiated since the
	// kernel thread yields from the kernel main loop while a user thread
	// yields from within a syscall, i.e. from within a signal context.
	if(this_t->parent->pid != 0)	yield_user(this_t);
	else							yield_kernel();

	return 0;
}

sc_t *x86_sc_arg(thread_t *this_t){
	x86_hw_op_t *op;
	thread_ctx_t *ctx;
	sc_t sc;


	ctx = stack_top(this_t->ctx_stack);
	ctx->sc_op = *x86_int_op();
	op = &ctx->sc_op;

	if(op->int_ctrl.num != INT_SYSCALL)
		LNX_EEXIT("hardware-op not a syscall\n");

	LNX_DEBUG("syscall(thread = %s.%u, data = %p)\n",
		this_t->parent->name, this_t->tid,
		op->int_ctrl.payload
	);

	if(copy_from_user(&sc, op->int_ctrl.payload, sizeof(sc), this_t->parent) != 0)
		LNX_EEXIT("copy syscall args from user-space failed\n");

	LNX_DEBUG("syscall(num = %u, param = %p, psize = %u)\n", sc.num, sc.param, sc.size);

	if(x86_sc_overlay_call(sc.num, sc.param, OLOC_PRE, overlays) != 0)
		LNX_EEXIT("syscall overlay failed\n");

	return op->int_ctrl.payload;
}

void x86_sc_epilogue(thread_t *this_t){
	x86_hw_op_t *op;
	thread_ctx_t *ctx;
	sc_t sc;


	ctx = stack_top(this_t->ctx_stack);
	op = &ctx->sc_op;

	if(copy_from_user(&sc, op->int_ctrl.payload, sizeof(sc), this_t->parent) != 0)
		LNX_EEXIT("copy syscall args from user-space failed\n");

	if(sc.errnum == 0)
		sc.errnum = -x86_sc_overlay_call(sc.num, sc.param, OLOC_POST, overlays);

	if(copy_to_user(op->int_ctrl.payload, &sc, sizeof(sc_t), this_t->parent) != 0)
		LNX_EEXIT("copy syscall args to user-space failed\n");

	x86_hw_syscall_return();
}


/* local functions */
static void yield_user(thread_t *this_t){
	x86_sched_trigger();

	while(this_t->state != READY){
		lnx_pause();
	}

	x86_sched_force(this_t);
}

static void yield_kernel(void){
	thread_t *kernel;


	kernel = sched_running();

	// NOTE to improve performance the hardware would need
	//      to be notified of the thread change, since
	//      otherwise no syscalls from init are issued
	x86_sched_yield();
	x86_sched_wait(kernel);
}

static int overlay_exit(void *param){
	sc_exit_t kparam;


	if(copy_from_user(&kparam, param, sizeof(kparam), sched_running()->parent) != 0)
		return -errno;

	if(!kparam.kill_siblings)
		return 0;

	LNX_DEBUG("application exit with %d\n", kparam.status);

	if(x86_rootfs_dump() != 0){
		LNX_ERROR("brickos file system dump failed\n");
		kparam.status = -1;
	}

	x86_memory_cleanup();
	x86_std_fini();

	lnx_exit(kparam.status);

	return 0;
}

static int overlay_mmap(void *param){
	sc_fs_t kparam;
	devtree_memory_t *kheap;
	process_t *this_p;


	this_p = sched_running()->parent;
	kheap = (devtree_memory_t*)devtree_find_memory_by_name(&__dt_memory_root, "heap");

	if(copy_from_user(&kparam, param, sizeof(kparam), this_p) != 0)
		return -errno;

	// In order for mmap to work in the x86 setup the kernel heap is created
	// as a shared memory region. Hence, instead of returning the mmaped address
	// to the application, the offset relative to the kernel heap base is
	// returned, which is used on the application relative to its address of the
	// shared memory.
	if(kparam.payload < kheap->base || kparam.payload >= kheap->base + kheap->size){
		LNX_ERROR("trying to mmap non-heap address %p\n", kparam.payload);
		return_errno(E_INVAL);
	}

	kparam.payload -= (ptrdiff_t)kheap->base;

	return copy_to_user(param, &kparam, sizeof(kparam), this_p);
}
