#include <config/config.h>
#include <arch/arch.h>
#include <arch/core.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/memblock.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* local/static prototypes */
static int sc_hdlr_malloc(void *p);
static int sc_hdlr_free(void *p);


/* static variables */
static memblock_t *process_mem = 0x0;
static mutex_t umem_mtx = MUTEX_INITIALISER();


/* global functions */
void *umalloc(size_t n){
	void *p;


	mutex_lock(&umem_mtx);
	p = memblock_alloc(&process_mem, n, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&umem_mtx);

	return p;
}

void ufree(void *addr){
	mutex_lock(&umem_mtx);

	if(memblock_free(&process_mem, addr) < 0)
		kpanic(0x0, "double free at %p\n", addr);

	mutex_unlock(&umem_mtx);
}


/* local functions */
static int init(void){
	int r;


	r = E_OK;

	/* init memory area */
	process_mem = (void*)(CONFIG_KERNEL_PROC_BASE);

	if(memblock_init(process_mem, CONFIG_KERNEL_PROC_SIZE) < 0)
		r |= errno;

	/* register syscalls */

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

	mutex_lock(&this_p->mtx);
	list_add_tail(this_p->memory.pages, page);
	mutex_unlock(&this_p->mtx);

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
	list_rm(this_p->memory.pages, page);
	page_free(this_p, page);

	mutex_unlock(&this_p->mtx);

	return E_OK;
}
