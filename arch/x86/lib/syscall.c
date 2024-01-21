/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/sched.h>
#include <sys/atomic.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/devicetree.h>
#include <sys/memblock.h>
#include <sys/thread.h>
#include <sys/syscall.h>
#include <sys/string.h>


/* types */
typedef struct{
	char const *name;
	int (*hdlr)(x86_hw_op_t *op);

	unsigned int cnt;
} ops_t;


/* local/static prototypes */
// hardware event handling
static void hw_event_hdlr(int sig);
static void usignal_hdlr(int sig);

static int event_syscall_return(x86_hw_op_t *op);
static int event_copy_from_user(x86_hw_op_t *op);
static int event_copy_to_user(x86_hw_op_t *op);
static int event_setup(x86_hw_op_t *op);
static int event_signal(x86_hw_op_t *op);
static int event_inval(x86_hw_op_t *op);

// syscall overlays
static int overlay_malloc(void *param);
static int overlay_free(void *param);

static int overlay_thread_create(void *param);

static int overlay_sigregister(void *param);

static int overlay_mmap(void *param);

static void usignal_issue(char const *from);


/* static variables */
// hardware event handling
static int volatile syscall_return_pending = 0;

static ops_t ops[] = {
	{ .name = "int_trigger",	.hdlr = event_inval },
	{ .name = "int_return",		.hdlr = event_inval },
	{ .name = "int_set",		.hdlr = event_inval },
	{ .name = "syscall_return",	.hdlr = event_syscall_return },
	{ .name = "copy_from_user",	.hdlr = event_copy_from_user },
	{ .name = "copy_to_user",	.hdlr = event_copy_to_user },
	{ .name = "uart_config",	.hdlr = event_inval },
	{ .name = "display_config",	.hdlr = event_inval },
	{ .name = "setup",			.hdlr = event_setup },
	{ .name = "signal",			.hdlr = event_signal },
	{ .name = "invalid",		.hdlr = event_inval },
};

STATIC_ASSERT(sizeof_array(ops) == HWO_NOPS + 1);

// syscall overlays
static x86_sc_overlay_t overlays[] = {
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = overlay_mmap,				.loc = OLOC_POST },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = overlay_malloc,			.loc = OLOC_POST },
	{ .call = overlay_free,				.loc = OLOC_PRE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = overlay_thread_create,	.loc = OLOC_POST },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = overlay_sigregister,		.loc = OLOC_POST },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
};

STATIC_ASSERT(sizeof_array(overlays) == NSYSCALLS);

// memory/heap overlay data
static uint8_t mem_blob[DEVTREE_HEAP_SIZE];
static memblock_t *app_heap = 0x0;

static int kheap_shmid = -1;
static void *kheap_base = 0x0;

// signal overlay data
static thread_entry_t sig_hdlr = 0x0;
static bool usignal_pending = false;
static void *usignal_arg = 0x0;


/* global functions */
int x86_sc(sc_num_t num, void *param, size_t psize){
	sc_t sc;


	if(x86_sc_overlay_call(num, param, OLOC_PRE, overlays) != 0)
		return -errno;

	/* trigger syscall */
	// set pending flag
	if(cas((int*)(&syscall_return_pending), 0, 1) != 0)
		LNX_EEXIT("there must not be any pending syscalls\n");

	// trigger syscall interrupt
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errnum = E_UNKNOWN;

	LNX_DEBUG("syscall(num = %d, param = %p, psize = %u, data = %p)\n",
		num,
		param,
		psize,
		&sc
	);

	x86_hw_int_trigger(INT_SYSCALL, &sc);

	// wait for interrupt return
	LNX_DEBUG("waiting for syscall return\n");

	while(syscall_return_pending){
		lnx_nanosleep(500000);
	}

	/* check pending signals */
	if(usignal_pending)
		usignal_issue("syscall");

	/* post processing */
	LNX_DEBUG("syscall errno: %d\n", sc.errnum);

	if(sc.errnum)
		return_errno(sc.errnum);

	return x86_sc_overlay_call(num, param, OLOC_POST, overlays);
}


/* local functions */
static int init(void){
	lnx_sigset_t blocked;


	/* register signals */
	lnx_sigaction(CONFIG_X86EMU_USIGNAL_SIG, usignal_hdlr, 0x0);

	// Ensure a hardware operation cannot be interrupted by the signal
	// used to implement the brickos usignal mechanism. This is needed
	// since the usignal signal is triggered from within a hardware
	// operation, cf. event_signal(), which again might use syscalls,
	// i.e. triggering hardware operations, which would then block the
	// process since the second hardware operation cannot be started
	// while the first one has not been finished.
	memset(&blocked, 0x0, sizeof(lnx_sigset_t));
	lnx_sigaddset(&blocked, CONFIG_X86EMU_USIGNAL_SIG);
	lnx_sigaction(CONFIG_X86EMU_HW_SIG, hw_event_hdlr, &blocked);

	/* setup memory */
	app_heap = (void*)mem_blob;
	memblock_init(app_heap, DEVTREE_HEAP_SIZE);

	return -errno;
}

lib_init(0, init);

static void hw_event_hdlr(int sig){
	x86_hw_op_t op;


	x86_hw_op_read(&op);
	LNX_DEBUG("[%u] %s(tid = %u, num = %d)\n", op.seq, ops[op.num].name, op.tid, op.num);

	if(op.num >= HWO_NOPS)
		op.num = HWO_NOPS;

	op.retval = ops[op.num].hdlr(&op);

	LNX_DEBUG("  [%u] status: %s\n", op.seq, (op.retval == 0 ? "ok" : "error"));

	x86_hw_op_read_writeback(&op);
}

static void usignal_hdlr(int sig){
	void *arg = usignal_arg;


	usignal_pending = false;
	usignal_arg = 0x0;

	if(arg == 0x0)
		LNX_EEXIT("no usignal arguments set\n");

	LNX_DEBUG("handle usignal %ld\n", (long int)arg);
	sig_hdlr(arg);
	LNX_DEBUG("usignal handler completed\n");
}

static int event_syscall_return(x86_hw_op_t *op){
	if(cas((int*)(&syscall_return_pending), 1, 0) != 0)
		LNX_EEXIT("no pending syscall\n");

	return 0;
}

static int event_copy_from_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] addr = %p, size = %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_write(CONFIG_X86EMU_HW_PIPE_WR, op->copy.addr, op->copy.n);

	return 0;
}

static int event_copy_to_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] addr = %p, size = %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, op->copy.addr, op->copy.n);

	return 0;
}

static int event_setup(x86_hw_op_t *op){
	kheap_shmid = op->setup.shm_id;
	kheap_base = lnx_shmat(kheap_shmid);

	if(kheap_base == (void*)-1)
		LNX_EEXIT("unable to allocated kernel heap shared memory on id %d\n", kheap_shmid);

	return 0;
}

static int event_signal(x86_hw_op_t *op){
	// it has been ensured that CONFIG_X86EMU_USIGNAL_SIG cannot
	// interrupt the handler for the current hardware operation

	usignal_arg = op->signal.arg;

	if(syscall_return_pending){
		LNX_DEBUG("defer usignal %ld to syscall handler\n", (long int)usignal_arg);
		usignal_pending = true;
	}
	else
		usignal_issue("event-handler");

	return 0;
}

static int event_inval(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] invalid hardware-op\n", op->seq);

	return -1;
}

static int overlay_malloc(void *param){
	sc_malloc_t *p = (sc_malloc_t*)param;
	void *brickos_addr;


	if(p->size == 0)
		return 0;

	brickos_addr = p->p;

	p->p = memblock_alloc(&app_heap, p->size + sizeof(brickos_addr), 8);

	memcpy(p->p, &brickos_addr, sizeof(brickos_addr));
	p->p += sizeof(brickos_addr);

	if(p->p == 0x0)
		return_errno(E_NOMEM);

	return 0;
}

static int overlay_free(void *param){
	sc_malloc_t *p = (sc_malloc_t*)param;
	void *brickos_addr;


	p->p -= sizeof(brickos_addr);
	memcpy(&brickos_addr, p->p, sizeof(brickos_addr));

	if(memblock_free(&app_heap, p->p) < 0)
		LNX_EEXIT("double free at %p\n", p->p);

	p->p = brickos_addr;

	return 0;
}

static int overlay_thread_create(void *param){
	sc_thread_t *p = (sc_thread_t*)param;


	sched_yield();

	x86_hw_op_active_tid = p->tid;
	_exit(p->entry(p->arg), false);
	x86_hw_op_active_tid = 0;

	return 0;
}

static int overlay_sigregister(void *param){
	sig_hdlr = ((sc_signal_t*)param)->hdlr;

	return 0;
}

static int overlay_mmap(void *param){
	sc_fs_t *p = (sc_fs_t*)param;


	// for a description of the x86 mmap overlay mechanism refer
	// to the documentation of the mmap overlay within the kernel
	p->payload += (ptrdiff_t)kheap_base;

	return 0;
}

static void usignal_issue(char const *from){
	LNX_DEBUG("issue usignal %ld from %s\n", (long int)usignal_arg, from);
	lnx_kill(lnx_getpid(), CONFIG_X86EMU_USIGNAL_SIG);
}
