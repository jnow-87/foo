/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_H
#define ARCH_H


#include BUILD_ARCH_HEADER


/* macros */
#ifdef BUILD_KERNEL

#define arch_kernel_call(p, err_ret) \
	(arch_cbs_kernel.p == 0) ? (err_ret) : arch_cbs_kernel.p

#endif // BUILD_KERNEL

#define arch_common_call(p, err_ret) \
	(arch_cbs_common.p == 0) ? (err_ret) : arch_cbs_common.p

#define arch_info(c) \
	arch_info.c


#endif // ARCH_H
