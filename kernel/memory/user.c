/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/memblock.h>
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
	process_mem = (void*)(CONFIG_KERNEL_PROC_BASE);
	memblock_init(process_mem, CONFIG_KERNEL_PROC_SIZE);

	return -errno;
}

kernel_init(0, init);
