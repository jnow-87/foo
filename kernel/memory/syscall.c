/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/memory.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/mutex.h>
#include <sys/list.h>


/* local/static prototypes */
static int sc_hdlr_malloc(void *p);
static int sc_hdlr_free(void *p);


/* local functions */
static int init(void){
	int r;


	r = E_OK;

	r |= sc_register(SC_MALLOC, sc_hdlr_malloc);
	r |= sc_register(SC_FREE, sc_hdlr_free);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_malloc(void *_p){
	page_size_t psize;
	sc_malloc_t *p;
	page_t *page;
	process_t *this_p;


	p = (sc_malloc_t*)_p;
	this_p = sched_running()->parent;

	/* adjust size to page boundary requirements */
#ifdef CONFIG_KERNEL_VIRT_MEM
	for(psize = PAGESIZE_MIN; psize<=PAGESIZE_MAX; psize++){
		if(p->size <= PAGESIZE_BYTES(psize))
			break;
	}

	if(psize > PAGESIZE_MAX)
		return_errno(E_LIMIT);
#else // CONFIG_KERNEL_VIRT_MEM
	psize = p->size;
#endif // CONFIG_KERNEL_VIRT_MEM

	/* allocate memory */
	page = page_alloc(this_p, psize);

	if(page == 0x0)
		return_errno(E_NOMEM);

	/* prepare result */
#ifdef CONFIG_KERNEL_VIRT_MEM
	p->p = page->virt_addr;
	p->size = PAGESIZE_BYTES(psize);
#else // CONFIG_KERNEL_VIRT_MEM
	p->p = page->phys_addr;
	p->size = psize;
#endif // CONFIG_KERNEL_VIRT_MEM

	return E_OK;
}

static int sc_hdlr_free(void *_p){
	process_t *this_p;
	thread_t const *this_t;
	page_t *page;
	sc_malloc_t *p;


	p = (sc_malloc_t*)_p;
	this_t = sched_running();
	this_p = this_t->parent;

	mutex_lock(&this_p->mtx);

	/* get page */
#ifdef CONFIG_KERNEL_VIRT_MEM
	page = list_find(this_p->memory.pages, virt_addr, p->p);
#else // CONFIG_KERNEL_VIRT_MEM
	page = list_find(this_p->memory.pages, phys_addr, p->p);
#endif // CONFIG_KERNEL_VIRT_MEM

	if(page == 0x0)
		kpanic(this_t, "no page found to free\n");

	/* free page */
	page_free(this_p, page);

	mutex_unlock(&this_p->mtx);

	return E_OK;
}
