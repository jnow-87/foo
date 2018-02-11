#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <sys/syscall.h>
#include <sys/list.h>
#include "sched.h"


/* static/local prototypes */
static int sc_hdlr_process_create(void *p);
static int sc_hdlr_process_info(void *p);


/* local functions */
static int init(void){
	int r;


	/* register syscalls */
	r = E_OK;

	r |= sc_register(SC_PROCCREATE, sc_hdlr_process_create);
	r |= sc_register(SC_PROCINFO, sc_hdlr_process_info);

	return r;
}

kernel_init(2, init);

static int sc_hdlr_process_create(void *_p){
	sc_process_t *p = (sc_process_t*)_p;
	char name[p->name_len + 1];
	char args[p->args_len + 1];
	process_t *this_p,
			  *new;


	p->pid = 0;

	/* process arguments */
	this_p = sched_running()->parent;

	copy_from_user(name, p->name, p->name_len + 1, this_p);
	copy_from_user(args, p->args, p->args_len + 1, this_p);

	/* create process */
	DEBUG("create process \"%s\" with args \"%s\"\n", name, args);

	new = process_create(p->binary, p->bin_type, p->name, p->args, this_p->cwd);

	if(new == 0x0)
		return -errno;

	mutex_lock(&sched_mtx);
	sched_transition(list_first(new->threads), READY);
	mutex_unlock(&sched_mtx);

	p->pid = new->pid;

	return E_OK;
}

static int sc_hdlr_process_info(void *_p){
	sc_process_t *p;
	process_t *this_p;


	p = (sc_process_t*)_p;
	this_p = sched_running()->parent;

	p->pid = this_p->pid;

	return E_OK;
}
