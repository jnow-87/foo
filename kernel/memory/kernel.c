/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <sys/memblock.h>
#include <sys/devtree.h>
#include <sys/devicetree.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/string.h>


/* static variables */
static memblock_t *kernel_heap = 0x0;
static mutex_t kmem_mtx = NOINT_MUTEX_INITIALISER();


/* global functions */
void *kmalloc(size_t n){
	void *p;


	mutex_lock(&kmem_mtx);
	p = memblock_alloc(&kernel_heap, n, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&kmem_mtx);

	if(n != 0 && p == 0x0)
		set_errno(E_NOMEM);

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


	n *= size;
	p = kmalloc(n);

	if(p != 0x0)
		memset(p, 0, n);

	return p;
}

void kfree(void *addr){
	mutex_lock(&kmem_mtx);

	if(memblock_free(&kernel_heap, addr) < 0)
		kpanic("double free at %p\n", addr);

	mutex_unlock(&kmem_mtx);
}

void *kmmap(void *addr){
#ifdef CONFIG_KERNEL_VIRT_MEM
	// TODO
	set_errno(E_NOIMP);

	return 0x0;
#else
	return addr;
#endif // CONFIG_KERNEL_VIRT_MEM
}

void kmunmap(void *addr){
#ifdef CONFIG_KERNEL_VIRT_MEM
	// TODO
	set_errno(E_NOIMP);
#endif // CONFIG_KERNEL_VIRT_MEM
}

bool iskheap(void *addr){
	return (addr >= (void*)DEVTREE_HEAP_BASE) && addr < KERNEL_STACK(DEVTREE_ARCH_NCORES);
}

int copy_from_user(void *kernel, void const *user, size_t n, struct process_t *this_p){
	if(user == 0x0)
		return_errno(E_INVAL);

	if(arch_ops_kernel.copy_from_user != 0x0)
		return arch_ops_kernel.copy_from_user(kernel, user, n, this_p);

	memcpy(kernel, user, n);

	return 0;
}

int copy_to_user(void *user, void const *kernel, size_t n, struct process_t *this_p){
	if(user == 0x0)
		return_errno(E_INVAL);

	if(arch_ops_kernel.copy_to_user != 0x0)
		return arch_ops_kernel.copy_to_user(user, kernel, n, this_p);

	memcpy(user, kernel, n);

	return 0;
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
	kernel_heap = (void*)DEVTREE_HEAP_BASE;
	memblock_init(kernel_heap, (size_t)(KERNEL_STACK(DEVTREE_ARCH_NCORES) - DEVTREE_HEAP_BASE));

	return -errno;
}

kernel_init(0, init);
