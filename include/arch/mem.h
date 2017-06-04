#ifndef ARCH_MEM_H
#define ARCH_MEM_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define page_entry_write(p)					(arch_kernel_call(page_entry_write, E_OK)(p))
#define page_entry_inval_idx(idx, sync)		(arch_kernel_call(page_entry_inval_idx, E_OK)(idx, sync))
#define page_entry_inval_va(va, sync)		(arch_kernel_call(page_entry_inval_va, E_OK)(va, sync))
#define page_entry_search(p, r)				(arch_kernel_call(page_entry_search, E_OK)(p, r))
#define copy_from_user(tgt, src, size, p)	(arch_kernel_call(copy_from_user, E_OK)(tgt, src, size, p))
#define copy_to_user(tgt, src, size, p)		(arch_kernel_call(copy_to_user, E_OK)(tgt, src, size, p))


#endif // ARCH_MEM_H
