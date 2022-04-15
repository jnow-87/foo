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
#include <sys/memblock.h>
#include <sys/devtree.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/string.h>


/* static variables */
static devtree_memory_t const *kmem_dt_node;

static memblock_t *kernel_heap = 0x0;
static mutex_t kmem_mtx = NOINT_MUTEX_INITIALISER();


/* global functions */
void *kmalloc(size_t n){
	void *p;


	mutex_lock(&kmem_mtx);
	p = memblock_alloc(&kernel_heap, n, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&kmem_mtx);

	if(n != 0 && p == 0x0)
		errno = E_NOMEM;

	return p;
}

void *kpalloc(size_t n){
	void *p;


	p = kmalloc(n);

	if(p == 0x0)
		kpanic("out of memory\n");

	return p;
}

void *kcalloc(size_t n, size_t size){
	void *p;
	size_t x;


	x = n * size;
	p = kmalloc(x);

	if(p != 0x0)
		memset(p, 0, x);

	return p;
}

void kfree(void *addr){
	mutex_lock(&kmem_mtx);

	if(memblock_free(&kernel_heap, addr) < 0)
		kpanic("double free at %p\n", addr);

	mutex_unlock(&kmem_mtx);
}

bool iskheap(void *addr){
	return addr >= kmem_dt_node->base && addr < kmem_dt_node->base + kmem_dt_node->size;
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
	// NOTE the memory node is ensured to exist by the build system
	kmem_dt_node = devtree_find_memory_by_name(&__dt_memory_root, "kernel-heap");

	kernel_heap = kmem_dt_node->base;
	memblock_init(kernel_heap, kmem_dt_node->size);

	return -errno;
}

kernel_init(0, init);
