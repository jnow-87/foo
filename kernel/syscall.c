/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/syscall.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/devicetree.h>


/* local/static prototypes */
static void sc_hdlr(int_num_t num, void *payload);
static int syscall(sc_num_t num, void *param, size_t psize, thread_t *this_t);


/* static variables */
static sc_hdlr_t sc_map[NSYSCALLS] = { 0x0 };
static mutex_t sc_mtx = MUTEX_INITIALISER();


/* global functions */
int sc_register(sc_num_t num, sc_hdlr_t hdlr){
	mutex_lock(&sc_mtx);

	if(num >= NSYSCALLS)
		goto_errno(err, E_INVAL);

	if(sc_map[num] != 0x0)
		goto_errno(err, E_INUSE);

	sc_map[num] = hdlr;

	mutex_unlock(&sc_mtx);

	return 0;


err:
	mutex_unlock(&sc_mtx);

	return -errno;
}

int sc_release(sc_num_t num){
	if(num >= NSYSCALLS)
		return_errno(E_INVAL);

	mutex_lock(&sc_mtx);
	sc_map[num] = 0x0;
	mutex_unlock(&sc_mtx);

	return 0;
}


/* local functions */
static int init(void){
	return int_register(DEVTREE_ARCH_SYSCALL_INT, sc_hdlr, 0x0);
}

kernel_init(0, init);

static void sc_hdlr(int_num_t num, void *payload){
	thread_t *this_t;
	sc_t sc;
	sc_t *sc_user;


	this_t = sched_running();
	sc_user = sc_arg(this_t);

	if(copy_from_user(&sc, sc_user, sizeof(sc), this_t->parent) != 0)
		return;

	sc.errno = -syscall(sc.num, sc.param, sc.size, this_t);

	(void)copy_to_user(sc_user, &sc, sizeof(sc), this_t->parent);
}

static int syscall(sc_num_t num, void *param, size_t psize, thread_t *this_t){
	int r;
	char kparam[psize];
	sc_hdlr_t hdlr;


	mutex_lock(&sc_mtx);
	hdlr = sc_map[num];
	mutex_unlock(&sc_mtx);

	/* check syscall */
	if(num >= NSYSCALLS || hdlr == 0x0)
		return_errno(E_INVAL);

	/* copy arguments to kernel space */
	if(copy_from_user(kparam, param, psize, this_t->parent) != 0)
		return -errno;

	/* execute callback */
	int_enable(INT_ALL);
	r = hdlr(kparam);
	int_enable(INT_NONE);

	if(r != 0){
		if(errno == 0)
			set_errno(E_UNKNOWN);

		DEBUG("syscall %d on %s:%u failed \"%s\" (%#x)\n", num, this_t->parent->name, this_t->tid, strerror(errno), r);
	}
	else if(errno != 0)
		WARN("uncaught error for syscall %d: %s\n", (int)num, strerror(errno));

	/* copy result to user space */
	(void)copy_to_user(param, kparam, psize, this_t->parent);

	return -errno;
}
