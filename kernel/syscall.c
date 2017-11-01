#include <arch/mem.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/errno.h>


/* static variables */
sc_hdlr_t sc_map[NSYSCALLS] = { 0x0 };


/* global functions */
int sc_register(sc_t num, sc_hdlr_t hdlr){
	if(num >= NSYSCALLS)
		return_errno(E_INVAL);

	if(sc_map[num] != 0x0)
		return_errno(E_INUSE);

	DEBUG("register handler for syscall %d to %#x\n", num, hdlr);
	sc_map[num] = hdlr;

	return E_OK;
}

int sc_release(sc_t num){
	if(num >= NSYSCALLS)
		return_errno(E_INVAL);

	sc_map[num] = 0x0;

	return E_OK;
}

int ksc_hdlr(sc_t num, void *param, size_t psize){
	void *kparam;
	thread_t const *this_t;


	this_t = sched_running();

	DEBUG("handle syscall %d, data at: %#x with %u bytes\n", num, param, psize);

	/* check syscall */
	if(num >= NSYSCALLS)
		return_errno(E_INVAL);

	if(sc_map[num] == 0x0)
		return_errno(E_NOIMP);

	/* copy arguments to kernel space */
#ifdef CONFIG_KERNEL_VIRT_MEM
	kparam = kmalloc(psize);

	if(kparam == 0x0)
		return errno;

	if(copy_from_user(kparam, param, psize, this_t->parent) != E_OK)
		goto err;
#else
	kparam = param;
#endif // CONFIG_KERNEL_VIRT_MEM

	/* execute callback */
	if(sc_map[num](kparam, this_t) != E_OK)
		goto err;

	/* copy result to user space */
#ifdef CONFIG_KERNEL_VIRT_MEM
	if(copy_to_user(param, kparam, psize, this_t->parent) != E_OK)
		goto err;

	kfree(kparam);
#endif // CONFIG_KERNEL_VIRT_MEM

	return E_OK;


err:
#ifdef CONFIG_KERNEL_VIRT_MEM
	kfree(kparam);
#endif // CONFIG_KERNEL_VIRT_MEM

	return errno;
}
