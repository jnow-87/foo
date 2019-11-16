/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <kernel/csection.h>
#include <sys/memblock.h>
#include <sys/devtree.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static variables */
static memblock_t *kernel_heap = 0x0;
static csection_lock_t kmem_mtx = CSECTION_INITIALISER();


/* global functions */
void *kmalloc(size_t n){
	void *p;


	csection_lock(&kmem_mtx);
	p = memblock_alloc(&kernel_heap, n, CONFIG_KMALLOC_ALIGN);
	csection_unlock(&kmem_mtx);

	if(n != 0 && p == 0x0)
		errno = E_NOMEM;

	return p;
}

void kfree(void *addr){
	csection_lock(&kmem_mtx);

	if(memblock_free(&kernel_heap, addr) < 0)
		kpanic(0x0, "double free at %p\n", addr);

	csection_unlock(&kmem_mtx);
}


#ifdef CONFIG_KERNEL_VIRT_MEM

void *addr_virt_to_phys(void *va){
	// TODO
	return 0x0;
}

void *addr_phys_to_virt(void *pa){
	// TODO
	return 0x0;
}

#endif // CONFIG_KERNEL_VIRT_MEM


/* local functions */
static int init(void){
	devtree_memory_t const *node;


	// NOTE the memory node is ensured to exist by the build system
	node = devtree_find_memory_by_name(&__dt_memory_root, "kernel_heap");

	kernel_heap = node->base;
	memblock_init(kernel_heap, node->size);

	return -errno;
}

kernel_init(0, init);
