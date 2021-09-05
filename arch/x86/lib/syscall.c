/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <arch/atomic.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/sched.h>
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

typedef enum{
	OLOC_NONE = 0x0,
	OLOC_PRE = 0x1,
	OLOC_POST = 0x2,
} overlay_location_t;

typedef struct{
	int (*call)(void *);
	overlay_location_t loc;
} overlay_t;


/* local/static prototypes */
// hardware event handling
static void hw_event_hdlr(int sig);

static int event_syscall_return(x86_hw_op_t *op);
static int event_copy_from_user(x86_hw_op_t *op);
static int event_copy_to_user(x86_hw_op_t *op);
static int event_inval(x86_hw_op_t *op);

// syscall overlays
static int overlay_call(sc_num_t num, void *param, overlay_location_t loc);

static int overlay_malloc(void *p);
static int overlay_free(void *p);

static int overlay_thread_create(void *p);

static int overlay_sigregister(void *p);
static int overlay_sigsend(void *p);


/* static variables */
// hardware event handling
static int volatile syscall_return_pending = 0;

static ops_t ops[] = {
	{ .name = "exit",			.hdlr = event_inval },
	{ .name = "int_trigger",	.hdlr = event_inval },
	{ .name = "int_return",		.hdlr = event_inval },
	{ .name = "int_set",		.hdlr = event_inval },
	{ .name = "int_state",		.hdlr = event_inval },
	{ .name = "syscall_return",	.hdlr = event_syscall_return },
	{ .name = "copy_from_user",	.hdlr = event_copy_from_user },
	{ .name = "copy_to_user",	.hdlr = event_copy_to_user },
	{ .name = "uart_config",	.hdlr = event_inval },
	{ .name = "invalid",		.hdlr = event_inval },
};

// syscall overlays
static overlay_t overlays[] = {
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
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
	{ .call = overlay_sigsend,			.loc = OLOC_POST },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
	{ .call = 0x0,						.loc = OLOC_NONE },
};

// memory/heap overlay data
static uint8_t mem_blob[DEVTREE_APP_HEAP_SIZE];
static memblock_t *app_heap = 0x0;

// signal overlay data
static thread_entry_t sig_hdlr = 0x0;
static bool ignore_exit = false;


/* global functions */
int x86_sc(sc_num_t num, void *param, size_t psize){
	sc_t sc;


	if(ignore_exit && num == SC_EXIT)
		return E_OK;

	if(overlay_call(num, param, OLOC_PRE) != 0)
		return -errno;

	/* trigger syscall */
	// set pending flag
	if(cas((int*)(&syscall_return_pending), 0, 1) != 0)
		LNX_EEXIT("there must not be any pending syscalls\n");

	// trigger syscall interrupt
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errno = E_UNKNOWN;

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

	/* post processing */
	LNX_DEBUG("syscall errno: %d\n", sc.errno);

	if(sc.errno)
		return_errno(sc.errno);

	return overlay_call(num, param, OLOC_POST);
}


/* local functions */
static int init(void){
	lnx_sigaction(CONFIG_TEST_INT_HW_SIG, hw_event_hdlr, 0x0);

	app_heap = (void*)mem_blob;
	memblock_init(app_heap, DEVTREE_APP_HEAP_SIZE);

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

static int event_syscall_return(x86_hw_op_t *op){
	if(cas((int*)(&syscall_return_pending), 1, 0) != 0)
		LNX_EEXIT("no pending syscall\n");

	return 0;
}

static int event_copy_from_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] addr = %p, size = %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_write(CONFIG_TEST_INT_HW_PIPE_WR, op->copy.addr, op->copy.n);

	return 0;
}

static int event_copy_to_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] addr = %p, size = %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_read_fix(CONFIG_TEST_INT_HW_PIPE_RD, op->copy.addr, op->copy.n);

	return 0;
}

static int event_inval(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] invalid hardware-op\n", op->seq);

	return -1;
}

static int overlay_call(sc_num_t num, void *param, overlay_location_t loc){
	BUILD_ASSERT(sizeof_array(overlays) == NSYSCALLS);

	if(overlays[num].call == 0x0 || (overlays[num].loc & loc) == 0)
		return 0;

	return overlays[num].call(param);
}

static int overlay_malloc(void *_p){
	void *brickos_addr;
	sc_malloc_t *p;


	p = (sc_malloc_t*)_p;

	if(p->size == 0)
		return E_OK;

	brickos_addr = p->p;

	p->p = memblock_alloc(&app_heap, p->size + sizeof(brickos_addr), 8);

	memcpy(p->p, &brickos_addr, sizeof(brickos_addr));
	p->p += sizeof(brickos_addr);

	if(p->p == 0x0)
		return_errno(E_NOMEM);

	return E_OK;
}

static int overlay_free(void *_p){
	void *brickos_addr;
	sc_malloc_t *p;


	p = (sc_malloc_t*)_p;

	p->p -= sizeof(brickos_addr);
	memcpy(&brickos_addr, p->p, sizeof(brickos_addr));

	if(memblock_free(&app_heap, p->p) < 0)
		LNX_EEXIT("double free at %p\n", p->p);

	p->p = brickos_addr;

	return E_OK;
}

static int overlay_thread_create(void *_p){
	sc_thread_t *p;


	p = (sc_thread_t*)_p;

	sched_yield();

	x86_hw_op_active_tid = p->tid;
	_exit(p->entry(p->arg), false);
	x86_hw_op_active_tid = 0;

	return E_OK;
}

static int overlay_sigregister(void *p){
	sig_hdlr = ((sc_signal_t*)p)->hdlr;

	return E_OK;
}

static int overlay_sigsend(void *_p){
	sc_signal_t *p;


	p = (sc_signal_t*)_p;

	if(sig_hdlr == 0x0)
		LNX_EEXIT("signal handler not set\n");

	ignore_exit = true;
	x86_hw_op_active_tid = p->tid;

	(void)sig_hdlr((void*)p->sig);

	x86_hw_op_active_tid = 0;
	ignore_exit = false;

	return E_OK;
}
