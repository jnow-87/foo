#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/memblock.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static variables */
static memblock_t *kernel_heap = 0x0;
static mutex_t kmem_mtx = MUTEX_INITIALISER();


/* global functions */
void *kmalloc(size_t n){
	void *p;


	mutex_lock(&kmem_mtx);
	p = memblock_alloc(&kernel_heap, n, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&kmem_mtx);

	return p;
}

void kfree(void *addr){
	mutex_lock(&kmem_mtx);

	if(memblock_free(&kernel_heap, addr) < 0)
		kpanic(0x0, "double free at %p\n", addr);

	mutex_unlock(&kmem_mtx);
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
	kernel_heap = (void*)(CONFIG_KERNEL_HEAP_BASE);

	return memblock_init(kernel_heap, CONFIG_KERNEL_HEAP_SIZE);
}

kernel_init(0, init);
