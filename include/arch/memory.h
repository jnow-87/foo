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
#include <sys/errno.h>
#include <sys/string.h>


/* macros */
#define page_entry_write(p)					(arch_kernel_call(page_entry_write, 0)(p))
#define page_entry_inval_idx(idx, sync)		(arch_kernel_call(page_entry_inval_idx, 0)(idx, sync))
#define page_entry_inval_va(va, sync)		(arch_kernel_call(page_entry_inval_va, 0)(va, sync))
#define page_entry_search(p, r)				(arch_kernel_call(page_entry_search, 0)(p, r))
#define copy_from_user(tgt, src, size, p) \
	(arch_ops_kernel.copy_from_user != 0x0 ? arch_ops_kernel.copy_from_user(tgt, src, size, p) : (void)memcpy(tgt, src, size))

#define copy_to_user(tgt, src, size, p)	\
	(arch_ops_kernel.copy_to_user != 0x0 ? arch_ops_kernel.copy_to_user(tgt, src, size, p) : (void)memcpy(tgt, src, size))


#endif // ARCH_MEM_H
