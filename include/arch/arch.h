/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_H
#define ARCH_H


#include BUILD_ARCH_HEADER
#include <sys/devicetree.h>

#ifndef ASM
# ifndef BUILD_HOST
#  include <sys/errno.h>
# endif // BUILD_HOST
#endif // ASM


/* macros */
// call macros
#ifdef BUILD_KERNEL
# define arch_kernel_call(p, err_ret) \
	(arch_ops_kernel.p == 0x0) ? (err_ret) : arch_ops_kernel.p
#endif // BUILD_KERNEL

#define arch_common_call(p, err_ret) \
	(arch_ops_common.p == 0x0) ? (err_ret) : arch_ops_common.p

// kernel ops
#ifdef BUILD_KERNEL
# define core_id()							(arch_kernel_call(core_id, 0)())
# define core_sleep()						(arch_kernel_call(core_sleep, -E_NOIMP)())
# define core_panic(ctx)					(arch_kernel_call(core_panic, -E_NOIMP)(ctx))

# define PIR								core_id()

# ifdef DEVTREE_ARCH_MULTI_CORE
#  define cores_boot()						(arch_kernel_call(cores_boot, 0)())
# else
#  define cores_boot()						{}
# endif // DEVTREE_ARCH_MULTI_CORE

# define page_entry_write(p)				(arch_kernel_call(page_entry_write, 0)(p))
# define page_entry_inval_idx(idx, sync)	(arch_kernel_call(page_entry_inval_idx, 0)(idx, sync))
# define page_entry_inval_va(va, sync)		(arch_kernel_call(page_entry_inval_va, 0)(va, sync))
# define page_entry_search(p, r)			(arch_kernel_call(page_entry_search, 0)(p, r))

# define int_enable(en)						(arch_kernel_call(int_enable, false)(en))
# define int_enabled()						(arch_kernel_call(int_enabled, false)())

# define ipi_int(core, bcast, msg)			(arch_kernel_call(ipi_int, -E_NOIMP)(core, bcast, msg))
# define ipi_arg()							(arch_kernel_call(ipi_arg, 0x0)())

# define thread_ctx_init(ctx, this_t, entry, arg) \
	(arch_kernel_call(thread_ctx_init, 0x0)(ctx, this_t, entry, arg))

# define sc_arg(this_t)						(arch_kernel_call(sc_arg, 0x0)(this_t))
#endif // BUILD_KERNEL

// common ops
#define atomic(op, param)					(arch_common_call(atomic, -E_NOIMP)(op, param))

#define sc(num, param)						(arch_common_call(sc, -E_NOIMP)(num, param, sizeof(*param)))


#endif // ARCH_H
