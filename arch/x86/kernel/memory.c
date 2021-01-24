/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/process.h>
#include <sys/devicetree.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* static variables */
static uint8_t kernel_heap[DEVTREE_KERNEL_HEAP_SIZE] __used __section(".memory_kernel_heap");
static uint8_t kernel_stack[DEVTREE_KERNEL_STACK_SIZE] __used __section(".memory_kernel_stack");
static uint8_t app_heap[DEVTREE_APP_HEAP_SIZE] __used __section(".memory_app_heap");


/* global functions */
void x86_copy_from_user(void *target, void const *src, unsigned int n, struct process_t const *this_p){
	x86_hw_op_t op;


	op.num = HWO_COPY_FROM_USER;
	op.copy.addr = (void*)src;
	op.copy.n = n;

	LNX_DEBUG("copy-from: %p %d\n", src, n);

	x86_hw_op_write(&op);
	lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, target, n);
	x86_hw_op_write_writeback(&op);

	LNX_DEBUG("  status: %d\n", op.retval);
}

void x86_copy_to_user(void *target, void const *src, unsigned int n, struct process_t const *this_p){
	x86_hw_op_t op;


	op.num = HWO_COPY_TO_USER;
	op.copy.addr = target;
	op.copy.n = n;

	LNX_DEBUG("copy-to: %p %d\n", target, n);

	x86_hw_op_write(&op);
	lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, src, n);
	x86_hw_op_write_writeback(&op);

	LNX_DEBUG("  status: %d\n", op.retval);
}
