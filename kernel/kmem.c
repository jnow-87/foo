#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/kmutex.h>
#include <sys/types.h>
#include <sys/memblock.h>


/* static variables */
static memblock_t *kernel_heap = 0x0;
static kmutex_t kmem_mutex = KMUTEX_INITIALISER();


/* global functions */
void *kmalloc(size_t n){
	void *p;


	kmutex_lock(&kmem_mutex);
	p = memblock_alloc(&kernel_heap, n);
	kmutex_unlock(&kmem_mutex);

	return p;
}

void kfree(void *addr){
	kmutex_lock(&kmem_mutex);
	memblock_free(&kernel_heap, addr);
	kmutex_unlock(&kmem_mutex);
}

void *addr_virt_to_phys(void *va){
	// TODO
	return 0x0;
}

void *addr_phys_to_virt(void *pa){
	// TODO
	return 0x0;
}


/* local functions */
static error_t kmem_init(void){
	kernel_heap = (void*)(KERNEL_HEAP_BASE);
	memblock_init(kernel_heap, KERNEL_HEAP_SIZE);

	return E_OK;
}

kernel_init(0, kmem_init);
