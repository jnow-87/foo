#ifndef ARCH_MEM_H
#define ARCH_MEM_H


#include <arch/arch.h>
#include <kernel/kmem.h>
#include <sys/errno.h>
#include <sys/string.h>


/* macros */
#define page_entry_write(p)					(arch_kernel_call(page_entry_write, E_OK)(p))
#define page_entry_inval_idx(idx, sync)		(arch_kernel_call(page_entry_inval_idx, E_OK)(idx, sync))
#define page_entry_inval_va(va, sync)		(arch_kernel_call(page_entry_inval_va, E_OK)(va, sync))
#define page_entry_search(p, r)				(arch_kernel_call(page_entry_search, E_OK)(p, r))
#define copy_from_user(tgt, src, size, p) \
	(arch_cbs_kernel.copy_from_user != 0x0 ? arch_cbs_kernel.copy_from_user(tgt, src, size, p) : (memcpy(tgt, src, size), E_OK))

#define copy_to_user(tgt, src, size, p)	\
	(arch_cbs_kernel.copy_to_user != 0x0 ? arch_cbs_kernel.copy_to_user(tgt, src, size, p) : (memcpy(tgt, src, size), E_OK))


#endif // ARCH_MEM_H
