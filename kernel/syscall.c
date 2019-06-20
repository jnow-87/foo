/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/memory.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/string.h>


/* static variables */
static sc_hdlr_t sc_map[NSYSCALLS] = { 0x0 };
static mutex_t sc_mtx = MUTEX_INITIALISER();


/* global functions */
int sc_register(sc_t num, sc_hdlr_t hdlr){
	mutex_lock(&sc_mtx);

	if(num >= NSYSCALLS)
		goto_errno(err, E_INVAL);

	if(sc_map[num] != 0x0)
		goto_errno(err, E_INUSE);

	sc_map[num] = hdlr;

	mutex_unlock(&sc_mtx);

	return E_OK;


err:
	mutex_unlock(&sc_mtx);

	return -errno;
}

int sc_release(sc_t num){
	if(num >= NSYSCALLS)
		return_errno(E_INVAL);

	mutex_lock(&sc_mtx);
	sc_map[num] = 0x0;
	mutex_unlock(&sc_mtx);

	return E_OK;
}

/**
 * \brief	kernel system call handler
 * 			Identifying an executing the requested system call. During
 * 			system call execution interrupts are enabled.
 *
 * \param	num		system call number
 * \param	param	user space pointer to system call arguments
 * \param	psize	size of param
 *
 * \pre		errno is reset to E_OK
 */
void ksc_hdlr(sc_t num, void *param, size_t psize){
	int r;
	char kparam[psize];
	sc_hdlr_t hdlr;
	thread_t const *this_t;


	this_t = sched_running();

	mutex_lock(&sc_mtx);
	hdlr = sc_map[num];
	mutex_unlock(&sc_mtx);

	/* check syscall */
	if(num >= NSYSCALLS || hdlr == 0x0){
		errno = E_INVAL;
		return;
	}

	/* copy arguments to kernel space */
	copy_from_user(kparam, param, psize, this_t->parent);

	/* execute callback */
	int_enable(INT_ALL);
	r = hdlr(kparam);
	int_enable(INT_NONE);

	if(r != 0){
		if(errno == E_OK)
			errno = E_UNKNOWN;

		DEBUG("syscall %d on %s:%u failed \"%s\" (%#x)\n", num, this_t->parent->name, this_t->tid, strerror(errno), r);
	}

	/* copy result to user space */
	copy_to_user(param, kparam, psize, this_t->parent);
}
