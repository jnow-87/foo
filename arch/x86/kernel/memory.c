/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/init.h>
#include <kernel/process.h>
#include <sys/devtree.h>
#include <sys/devicetree.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* static variables */
static int kheap_shmid = -1;


/* global functions */
int x86_copy_from_user(void *kernel, void const *user, size_t n, struct process_t const *this_p){
	x86_hw_op_t op;


	op.num = HWO_COPY_FROM_USER;
	op.copy.addr = (void*)user;
	op.copy.n = n;

	LNX_DEBUG("copy_from(addr = %p, size = %u)\n", user, n);

	x86_hw_op_write(&op);
	lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, kernel, n);
	x86_hw_op_write_writeback(&op);

	LNX_DEBUG("  status: %d\n", op.retval);

	return op.retval;
}

int x86_copy_to_user(void *user, void const *kernel, size_t n, struct process_t const *this_p){
	x86_hw_op_t op;


	op.num = HWO_COPY_TO_USER;
	op.copy.addr = user;
	op.copy.n = n;

	LNX_DEBUG("copy_to(addr = %p, size = %u)\n", user, n);

	x86_hw_op_write(&op);
	lnx_write(CONFIG_X86EMU_HW_PIPE_WR, kernel, n);
	x86_hw_op_write_writeback(&op);

	LNX_DEBUG("  status: %d\n", op.retval);

	return op.retval;
}

void x86_memory_cleanup(void){
	devtree_memory_t *kheap;


	kheap = (devtree_memory_t*)devtree_find_memory_by_name(&__dt_memory_root, "heap");
	lnx_shmrm(kheap_shmid, kheap->base);
}


/* local functions */
static int init(void){
	devtree_memory_t *kheap;


	/* update kernel heap base address with a shared memory section */
	// NOTE kernel heap needs to be allocated as shared memory to
	//	    allow access by the application which is needed to implement
	//	    the mmap syscall overlays
	kheap = (devtree_memory_t*)devtree_find_memory_by_name(&__dt_memory_root, "heap");

	kheap_shmid = lnx_shmget(kheap->size);

	if(kheap_shmid < 0)
		LNX_EEXIT("kernel heap allocation failed in shmget()\n");

	kheap->base = lnx_shmat(kheap_shmid);

	if(kheap->base == (void*)-1)
		LNX_EEXIT("kernel heap allocation failed in shmat()\n");

	/* send shared memory id to the application */
	x86_hw_setup_complete(kheap_shmid);

	return 0;
}

platform_init(0, first, init);
