/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/devicetree.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* static variables */
static uint8_t kernel_heap[DEVTREE_KERNEL_HEAP_SIZE] __used __section(".memory_kernel_heap");
static uint8_t kernel_stack[DEVTREE_KERNEL_STACK_SIZE] __used __section(".memory_kernel_stack");
static uint8_t app_heap[DEVTREE_APP_HEAP_SIZE] __used __section(".memory_app_heap");
