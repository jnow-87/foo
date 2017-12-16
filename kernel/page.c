#include <config/config.h>
#include <kernel/process.h>
#include <kernel/page.h>
#include <kernel/kmem.h>
#include <kernel/umem.h>
#include <kernel/lock.h>
#include <kernel/panic.h>
#include <sys/memblock.h>
#include <sys/errno.h>


/* global functions */
page_t *page_alloc(struct process_t *this_p, page_size_t psize){
	page_t *page;
	size_t size;


	/* allocate page structure */
	page = kmalloc(sizeof(page_t));

	if(page == 0)
		goto_errno(err_0, E_NOMEM);

#if defined(CONFIG_KERNEL_VIRT_MEM)
	if(psize < PAGESIZE_MIN || psize > PAGESIZE_MAX)
		goto_errno(err_0, E_INVAL);

	size = PAGESIZE_BYTES(psize);
#else // CONFIG_KERNEL_VIRT_MEM
	size = psize;
#endif // CONFIG_KERNEL_VIRT_MEM

	/* set page attributes */
#ifdef CONFIG_KERNEL_VIRT_MEM
	page->psize = psize;
	page->flags = PAGE_COHERENT;
	page->priv = PAGE_USER;
	page->perm = PAGE_R | PAGE_W | PAGE_X;
	page->idx = 0;
#endif // CONFIG_KERNEL_VIRT_MEM

	/* allocate physical block */
	page->phys_addr = umalloc(size);

	if(page->phys_addr == 0)
		goto_errno(err_1, E_NOMEM);

	/* allocate virtual block */
#ifdef CONFIG_KERNEL_VIRT_MEM
	klock();
	page->virt_addr = memblock_alloc(&(this_p->memory.addr_space), size);
	kunlock();

	if(page->virt_addr == 0)
		goto_errno(err_2, E_NOMEM);
#endif // CONFIG_KERNEL_VIRT_MEM

	return page;


#ifdef CONFIG_KERNEL_VIRT_MEM
err_2:
	ufree(page->phys_addr);
#endif // CONFIG_KERNEL_VIRT_MEM

err_1:
	kfree(page);

err_0:
	return 0;
}

void page_free(struct process_t *this_p, page_t *page){
	/* free virtual block */
#ifdef CONFIG_KERNEL_VIRT_MEM
	klock();

	if(memblock_free(&(this_p->memory_space), page->virt_addr) < 0)
		kpanic(0x0, "double free at %p\n", page->virt_addr);

	kunlock();
#endif // CONFIG_KERNEL_VIRT_MEM

	/* free physica block */
	ufree(page->phys_addr);

	/* free page structure */
	kfree(page);
}
