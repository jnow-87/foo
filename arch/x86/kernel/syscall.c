/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/init.h>
#include <arch/x86/interrupt.h>
#include <arch/x86/rootfs.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/syscall.h>


/* local/static prototypes */
static void overlay_exit(void *param);


/* global functions */
int x86_sc(sc_num_t num, void *param, size_t psize){
	x86_hw_op_t op;


	if(num != SC_SCHEDYIELD)
		LNX_EEXIT("kernel assumed to only use SC_SCHEDYIELD\n");

	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = INT_SCHED;
	op.int_ctrl.data = 0x0;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	while(1){
		lnx_pause();

		if(sched_running()->parent->pid == 0)
			break;
	}

	return 0;
}


/* local functions */
static void sc_hdlr(int_num_t num, void *data){
	x86_hw_op_t *op;
	sc_t sc;
	thread_t const *this_t;


	this_t = sched_running();
	op = x86_int_op();

	if(op->int_ctrl.num != INT_SYSCALL)
		LNX_EEXIT("hardware-op not a syscall\n");

	LNX_DEBUG("syscall(thread = %s.%u, data = %p)\n",
		this_t->parent->name, this_t->tid,
		op->int_ctrl.data
	);

	copy_from_user(&sc, op->int_ctrl.data, sizeof(sc), this_t->parent);

	LNX_DEBUG("syscall(num = %u, param = %p, psize = %u)\n", sc.num, sc.param, sc.size);

	if(sc.num == SC_EXIT)
		overlay_exit(sc.param);

	/* call kernel syscall handler */
	ksc_hdlr(sc.num, sc.param, sc.size);

	/* set errno */
	LNX_DEBUG("errno: %d\n", errno);
	sc.errno = errno;

	copy_to_user(op->int_ctrl.data, &sc, sizeof(sc), this_t->parent);
}

static int init(void){
	return int_register(INT_SYSCALL, sc_hdlr, 0x0);
}

platform_init(0, init);

static void overlay_exit(void *param){
	sc_exit_t kparam;


	copy_from_user(&kparam, param, sizeof(kparam), sched_running()->parent);

	if(!kparam.kill_siblings)
		return;

	LNX_DEBUG("application exit with %d\n", kparam.status);

	if(x86_rootfs_dump() != 0){
		LNX_ERROR("brickos file system dump failed\n");
		kparam.status = -1;
	}

	x86_std_fini();

	lnx_exit(kparam.status);
}
