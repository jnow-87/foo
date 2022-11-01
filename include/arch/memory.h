/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_MEM_H
#define ARCH_MEM_H


#include <arch/arch.h>
#include <kernel/memory.h>


/* macros */
#define page_entry_write(p)					(arch_kernel_call(page_entry_write, 0)(p))
#define page_entry_inval_idx(idx, sync)		(arch_kernel_call(page_entry_inval_idx, 0)(idx, sync))
#define page_entry_inval_va(va, sync)		(arch_kernel_call(page_entry_inval_va, 0)(va, sync))
#define page_entry_search(p, r)				(arch_kernel_call(page_entry_search, 0)(p, r))


#endif // ARCH_MEM_H
