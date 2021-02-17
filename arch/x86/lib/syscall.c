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
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/devicetree.h>
#include <sys/memblock.h>
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

static int event_int_return(x86_hw_op_t *op);
static int event_copy_from_user(x86_hw_op_t *op);
static int event_copy_to_user(x86_hw_op_t *op);
static int event_inval(x86_hw_op_t *op);

// syscall overlays
static int overlay_malloc(sc_malloc_t *p);
static void overlay_free(sc_malloc_t *p);

static void overlay_thread_create(sc_thread_t *p);


/* static variables */
// hardware event handling
static int volatile syscall_return_pending = 0;

static ops_t ops[] = {
	{ .name = "exit",			.hdlr = event_inval },
	{ .name = "int trigger",	.hdlr = event_inval },
	{ .name = "int return",		.hdlr = event_int_return },
	{ .name = "int set",		.hdlr = event_inval },
	{ .name = "int state",		.hdlr = event_inval },
	{ .name = "copy from user",	.hdlr = event_copy_from_user },
	{ .name = "copy to user",	.hdlr = event_copy_to_user },
	{ .name = "invalid",		.hdlr = event_inval },
};

// memory/heap overlay data
static uint8_t mem_blob[DEVTREE_APP_HEAP_SIZE];
static memblock_t *app_heap = 0x0;


/* global functions */
int x86_sc(sc_t num, void *param, size_t psize){
	union{
		sc_arg_t arg;
		errno_t errno;
	} sc_data;
	x86_hw_op_t op;


	/* handle required overlays */
	if(num == SC_FREE)
		overlay_free(param);

	/* trigger syscall */
	// set pending flag
	if(cas((int*)(&syscall_return_pending), 0, 1) != 0)
		LNX_EEXIT("there must not be any pending syscalls\n");

	// trigger syscall interrupt
	sc_data.arg.num = num;
	sc_data.arg.param = param;
	sc_data.arg.size = psize;

	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = INT_SYSCALL;
	op.int_ctrl.data = &sc_data;

	LNX_DEBUG("syscall %d\n", num);
	LNX_DEBUG("  param: %p\n", param);
	LNX_DEBUG("  psize: %u\n", psize);
	LNX_DEBUG("   data: %p\n", &sc_data);

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	// wait for interrupt return
	LNX_DEBUG("waiting for int return\n");

	while(syscall_return_pending){
		lnx_nanosleep(500000);
	}

	/* post processing */
	LNX_DEBUG("syscall errno: %d\n", sc_data.errno);

	if(sc_data.errno != 0)
		return_errno(sc_data.errno);

	/* handle required overlays */
	if(num == SC_MALLOC)
		return overlay_malloc(param);

	if(num == SC_THREADCREATE)
		overlay_thread_create(param);

	return E_OK;
}


/* local functions */
static int init(void){
	lnx_sigset(CONFIG_TEST_INT_DATA_SIG, hw_event_hdlr);

	app_heap = (void*)mem_blob;
	memblock_init(app_heap, DEVTREE_APP_HEAP_SIZE);

	return -errno;
}

lib_init(0, init);

static void hw_event_hdlr(int sig){
	x86_hw_op_t op;


	x86_hw_op_read(&op);
	LNX_DEBUG("[%u] hardware-op\n", op.seq);
	LNX_DEBUG("  [%u] tid: %u\n", op.seq, op.tid);
	LNX_DEBUG("  [%u] event: %s (%d)\n", op.seq, ops[op.num].name, op.num);

	if(op.num >= HWO_NOPS)
		op.num = HWO_NOPS;

	op.retval = ops[op.num].hdlr(&op);

	LNX_DEBUG("  [%u] status: %s\n", op.seq, (op.retval == 0 ? "ok" : "error"));

	x86_hw_op_read_writeback(&op);
}

static int event_int_return(x86_hw_op_t *op){
	if(cas((int*)(&syscall_return_pending), 1, 0) != 0)
		LNX_EEXIT("no pending syscall\n");

	return 0;
}

static int event_copy_from_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] copy-from: %#x %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, op->copy.addr, op->copy.n);

	return 0;
}

static int event_copy_to_user(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] copy-to: %#x %u\n", op->seq, op->copy.addr, op->copy.n);
	lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, op->copy.addr, op->copy.n);

	return 0;
}

static int event_inval(x86_hw_op_t *op){
	LNX_DEBUG("  [%u] invalid hardware-op\n", op->seq);

	return -1;
}

static int overlay_malloc(sc_malloc_t *p){
	void *brickos_addr;


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

static void overlay_free(sc_malloc_t *p){
	void *brickos_addr;


	p->p -= sizeof(brickos_addr);
	memcpy(&brickos_addr, p->p, sizeof(brickos_addr));

	if(memblock_free(&app_heap, p->p) < 0)
		LNX_EEXIT("double free at %p\n", p->p);

	p->p = brickos_addr;
}

static void overlay_thread_create(sc_thread_t *p){
	sched_yield();

	x86_hw_op_active_tid = p->tid;
	_exit(p->entry(p->arg), false);
	x86_hw_op_active_tid = 0;
}
