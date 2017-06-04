#include <kernel/page.h>
#include <kernel/process.h>
#include <kernel/kmem.h>
#include <kernel/umem.h>
#include <sys/memblock.h>


/* global functions */
page_t *page_alloc(struct process_t *this_p, page_size_t psize){
	page_t *page;
	size_t size;


	/* allocate page structure */
	page = kmalloc(sizeof(page_t));

	if(page == 0)
		goto err_0;

	size = PAGESIZE_BYTES(psize);

	/* set page attributes */
	page->psize = psize;
	page->flags = PAGE_COHERENT;
	page->priv = PAGE_USER;
	page->perm = PAGE_R | PAGE_W | PAGE_X;
	page->idx = 0;

	/* allocate physical block */
	page->phys_addr = umalloc(size);

	if(page->phys_addr == 0)
		goto err_1;

	/* allocate virtual block */
	mutex_lock(&(this_p->memory.mtx));
	page->virt_addr = memblock_alloc(&(this_p->memory.addr_space), size);
	mutex_unlock(&(this_p->memory.mtx));

	if(page->virt_addr == 0)
		goto err_2;

	return page;


err_2:
	ufree(page->phys_addr);

err_1:
	kfree(page);

err_0:
	return 0;
}

void page_free(struct process_t *this_p, page_t *page){
	/* free virtual block */
	mutex_lock(&(this_p->memory.mtx));
	memblock_free(&(this_p->memory_space), page->virt_addr);
	mutex_unlock(&(this_p->memory.mtx));

	/* free physica block */
	ufree(page->phys_addr);

	/* free page structure */
	kfree(page);
}
