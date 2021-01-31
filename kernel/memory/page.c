/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <sys/memblock.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/list.h>


/* global functions */
page_t *page_alloc(struct process_t *this_p, page_size_t psize){
	page_t *page;
	size_t size;


	/* allocate page structure */
	page = kmalloc(sizeof(page_t));

	if(page == 0x0)
		goto err_0;

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

	if(page->phys_addr == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* allocate virtual block */
#ifdef CONFIG_KERNEL_VIRT_MEM
	mutex_lock(&this_p->mtx);
	page->virt_addr = memblock_alloc(&(this_p->memory.addr_space), size, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&this_p->mtx);

	if(page->virt_addr == 0x0)
		goto_errno(err_2, E_NOMEM);
#endif // CONFIG_KERNEL_VIRT_MEM

	/* update process page list */
	list_add_tail_safe(this_p->memory.pages, page, &this_p->mtx);

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
	mutex_lock(&this_p->mtx);

	list_rm(this_p->memory.pages, page);

	/* free virtual block */
#ifdef CONFIG_KERNEL_VIRT_MEM
	if(memblock_free(&(this_p->memory_space), page->virt_addr) < 0)
		kpanic("double free at %p\n", page->virt_addr);
#endif // CONFIG_KERNEL_VIRT_MEM

	mutex_unlock(&this_p->mtx);

	/* free physica block */
	ufree(page->phys_addr);

	/* free page structure */
	kfree(page);
}
