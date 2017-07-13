#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/kmutex.h>
#include <sys/types.h>
#include <sys/memblock.h>
#include <sys/errno.h>


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
static int umem_init(void){
	process_mem = (void*)(CONFIG_KERNEL_PROC_BASE);

	return memblock_init(process_mem, CONFIG_KERNEL_PROC_SIZE);
}

kernel_init(0, umem_init);
