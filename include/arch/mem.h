#ifndef ARCH_MEM_H
#define ARCH_MEM_H


#include <arch/arch.h>


/* macro */
#define page_entry_write(p) arch_page_entry_write(p)
#define arch_page_entry_write(p) \
	arch_kernel_call(page_entry_write, 0)(p)

#define page_entry_inval_idx(idx, sync) arch_page_entry_inval_idx(idx, sync)
#define arch_page_entry_inval_idx(idx, sync) \
	arch_kernel_call(page_entry_inval_idx, 0)(idx, sync)

#define page_entry_inval_va(va, sync) arch_page_entry_inval_va(va, sync)
#define arch_page_entry_inval_va(va, sync) \
	arch_kernel_call(page_entry_inval_va, 0)(va, sync)

#define page_entry_search(p, r) arch_page_entry_search(p, r)
#define arch_page_entry_search(p, r) \
	arch_kernel_call(page_entry_search, 0)(p, r)

#define copy_from_user(tgt, src, size, p) arch_copy_from_user(tgt, src, size, p)
#define arch_copy_from_user(tgt, src, size, p) \
	arch_kernel_call(copy_from_user, 0)(tgt, src, size, p)

#define copy_to_user(tgt, src, size, p) arch_copy_to_user(tgt, src, size, p)
#define arch_copy_to_user(tgt, src, size, p) \
	arch_kernel_call(copy_to_user, 0)(tgt, src, size, p)


#endif // ARCH_MEM_H
