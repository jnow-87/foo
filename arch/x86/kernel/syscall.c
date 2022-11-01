/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <arch/x86/init.h>
#include <arch/x86/syscall.h>
#include <arch/x86/interrupt.h>
#include <arch/x86/rootfs.h>
#include <arch/interrupt.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/compiler.h>
#include <sys/devtree.h>
#include <sys/types.h>
#include <sys/syscall.h>


/* local/static prototypes */
static void sc_hdlr(int_num_t num, void *payload);

static void yield_user(thread_t const *this_t);
static void yield_kernel(void);

static int overlay_exit(void *param);
static int overlay_mmap(void *param);


/* static variables */
// syscall overlays
static overlay_t overlays[] = {
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
};

STATIC_ASSERT(sizeof_array(overlays) == NSYSCALLS);


/* global functions */
int x86_sc(sc_num_t num, void *param, size_t psize){
	thread_t const *this_t;


	if(num != SC_SCHEDYIELD)
		LNX_EEXIT("kernel assumed to only use SC_SCHEDYIELD\n");

	this_t = sched_running();

	// Kernel and user thread yield calls need to be differentiated since the
	// kernel thread yields from the kernel main loop while a user thread
	// yields from within a syscall, i.e. from within a signal context. To
	// prevent issues with the active thread, scheduler and syscall interrupts
	// need to have the same priority, i.e. use the same x86 signal. Hence an
	// active syscall cannot be interrupted by a scheduler interrupt.
	if(this_t->parent->pid != 0)	yield_user(this_t);
	else							yield_kernel();

	return 0;
}


/* local functions */
static int init(void){
	return int_register(INT_SYSCALL, sc_hdlr, 0x0);
}

platform_init(0, init);

static void sc_hdlr(int_num_t num, void *payload){
	x86_hw_op_t op;
	sc_t sc;
	thread_t const *this_t;


	this_t = sched_running();
	op = *x86_int_op();

	if(op.int_ctrl.num != INT_SYSCALL)
		LNX_EEXIT("hardware-op not a syscall\n");

	LNX_DEBUG("syscall(thread = %s.%u, data = %p)\n",
		this_t->parent->name, this_t->tid,
		op.int_ctrl.payload
	);

	if(copy_from_user(&sc, op.int_ctrl.payload, sizeof(sc), this_t->parent) != 0)
		goto end;

	LNX_DEBUG("syscall(num = %u, param = %p, psize = %u)\n", sc.num, sc.param, sc.size);

	/* call kernel syscall handler */
	if(x86_sc_overlay_call(sc.num, sc.param, OLOC_PRE, overlays) == 0)
		sc_khdlr(sc.num, sc.param, sc.size);

	if(errno == 0)
		set_errno(x86_sc_overlay_call(sc.num, sc.param, OLOC_POST, overlays));

	/* set errno */
end:
	LNX_DEBUG("errno: %d\n", errno);
	sc.errno = errno;

	(void)copy_to_user(op.int_ctrl.payload, &sc, sizeof(sc), this_t->parent);

	op.num = HWO_SYSCALL_RETURN;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}


static void yield_user(thread_t const *this_t){
	int_type_t imask;


	/* wait for an interrupt */
	imask = int_enabled();

	// re-schedule
	sched_trigger();
	int_enable(imask);	// disabled by sched_trigger()

	// wait for this_t to become ready
	while(this_t->state != READY){
		lnx_pause();
	}

	/* re-schedule this_t */
	while(sched_running() != this_t){
		sched_trigger();
	}

	int_enable(imask);	// disabled by sched_trigger()
}

static void yield_kernel(void){
	x86_hw_int_trigger(INT_SCHED, 0x0);

	/* suspend till kernel is scheduled again */
	while(1){
		lnx_pause();

		if(sched_running()->parent->pid == 0)
			break;
	}
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
	kheap = (devtree_memory_t*)devtree_find_memory_by_name(&__dt_memory_root, "kernel-heap");

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
