#include <kernel/signal.h>
#include <kernel/sched.h>
#include <kernel/kmem.h>
#include <kernel/lock.h>
#include <sys/list.h>
#include <sys/errno.h>


/* global functions */
void ksignal_init(ksignal_t *sig){
	*sig = 0x0;
}

int ksignal_wait(ksignal_t *sig){
	ksignal_el_t *e;


	e = kmalloc(sizeof(ksignal_el_t));

	if(e == 0x0)
		return_errno(E_NOMEM);

	e->thread = sched_running();

	klock();
	list_add_tail(*sig, e);
	kunlock();

	sched_yield(WAITING);

	return E_OK;
}

int ksignal_send(ksignal_t *sig){
	ksignal_el_t *e;


	klock();
	e = list_first(*sig);

	if(e == 0x0)
		goto err_1;

	list_rm(*sig, e);
	kunlock();

	if(sched_enqueue((thread_t*)e->thread, READY) != E_OK)
		goto err_0;

	kfree(e);

	return E_OK;


err_1:
	kunlock();

err_0:
	return -errno;
}

int ksignal_bcast(ksignal_t *sig){
	ksignal_el_t *e;


	klock();

	list_for_each(*sig, e){
		list_rm(*sig, e);

		if(sched_enqueue((thread_t*)e->thread, READY) != E_OK)
			break;

		kfree(e);
	}

	kunlock();

	return -errno;
}
