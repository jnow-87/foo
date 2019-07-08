/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <sys/memblock.h>
#include <sys/devtree.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static variables */
static memblock_t *process_mem = 0x0;
static mutex_t umem_mtx = MUTEX_INITIALISER();


/* global functions */
void *umalloc(size_t n){
	void *p;


	mutex_lock(&umem_mtx);
	p = memblock_alloc(&process_mem, n, CONFIG_KMALLOC_ALIGN);
	mutex_unlock(&umem_mtx);

	return p;
}

void ufree(void *addr){
	mutex_lock(&umem_mtx);

	if(memblock_free(&process_mem, addr) < 0)
		kpanic(0x0, "double free at %p\n", addr);

	mutex_unlock(&umem_mtx);
}


/* local functions */
static int init(void){
	devtree_memory_t const *node;


	// NOTE the memory node is ensured to exist by the build system
	node = devtree_find_memory_by_name(&__dt_memory_root, "app_heap");

	process_mem = node->base;
	memblock_init(process_mem, node->size);

	return -errno;
}

kernel_init(0, init);
