#include <config/config.h>
#include <arch/arch.h>
#include <arch/core.h>
#include <kernel/init.h>
#include <kernel/kmutex.h>
#include <kernel/syscall.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/memblock.h>
#include <sys/errno.h>


/* local/static prototypes */
static int sc_hdlr_malloc(void *p, thread_t const *this_t);
static int sc_hdlr_free(void *p, thread_t const *this_t);


/* static variables */
static memblock_t *process_mem = 0x0;
static kmutex_t umem_mutex = KMUTEX_INITIALISER();


/* global functions */
void *umalloc(size_t n){
	void *p;


	kmutex_lock(&umem_mutex);
	p = memblock_alloc(&process_mem, n);
	kmutex_unlock(&umem_mutex);

	return p;
}

void ufree(void *addr){
	kmutex_lock(&umem_mutex);
	memblock_free(&process_mem, addr);
	kmutex_unlock(&umem_mutex);
}


/* local functions */
static int init(void){
	/* init memory area */
	process_mem = (void*)(CONFIG_KERNEL_PROC_BASE);

	if(memblock_init(process_mem, CONFIG_KERNEL_PROC_SIZE) < 0)
		goto err_0;

	/* register syscalls */
	if(sc_register(SC_MALLOC, sc_hdlr_malloc) < 0)
		goto err_0;

	if(sc_register(SC_FREE, sc_hdlr_free) < 0)
		goto err_1;

	return E_OK;


err_1:
	(void)sc_release(SC_MALLOC);

err_0:
	return errno;
}

kernel_init(0, init);

static int sc_hdlr_malloc(void *_p, thread_t const *this_t){
	page_size_t psize;
	sc_malloc_t *p;
	page_t *page;
	process_t *this_p;


	p = (sc_malloc_t*)_p;
	this_p = this_t->parent;

	/* adjust size to page boundary requirements */
#ifdef CONFIG_KERNEL_VIRT_MEM
	for(psize = PAGESIZE_MIN; psize<=PAGESIZE_MAX; psize++){
		if(p->size <= PAGESIZE_BYTES(psize))
			break;
	}

	if(psize > PAGESIZE_MAX)
		goto_errno(err, E_LIMIT);
#else // CONFIG_KERNEL_VIRT_MEM
	psize = p->size;
#endif // CONFIG_KERNEL_VIRT_MEM

	/* allocate memory */
	page = page_alloc(this_p, psize);

	if(page == 0x0)
		goto_errno(err, E_NOMEM);

	list_add_tail(this_p->memory.pages, page);

	/* prepare result */
#ifdef CONFIG_KERNEL_VIRT_MEM
	p->p = page->virt_addr;
	p->size = PAGESIZE_BYTES(psize);
#else // CONFIG_KERNEL_VIRT_MEM
	p->p = page->phys_addr;
	p->size = psize;
#endif // CONFIG_KERNEL_VIRT_MEM

	p->errno = E_OK;

	return E_OK;


err:
	p->errno = errno;
	return E_OK;
}

static int sc_hdlr_free(void *_p, thread_t const *this_t){
	process_t *this_p;
	page_t *page;
	sc_malloc_t *p;


	p = (sc_malloc_t*)_p;
	this_p = this_t->parent;

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

	p->errno = E_OK;

	return E_OK;
}
