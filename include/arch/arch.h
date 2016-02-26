#ifndef ARCH_H
#define ARCH_H


#include ARCH_HEADER


/* macros */
#ifdef KERNEL

#define arch_kernel_call(p, err_ret) \
	(arch_cbs_kernel.p == 0) ? (err_ret) : arch_cbs_kernel.p

#endif // KERNEL

#define arch_common_call(p, err_ret) \
	(arch_cbs_common.p == 0) ? (err_ret) : arch_cbs_common.p

#define arch_info(c) \
	arch_info.c


#endif // ARCH_H
