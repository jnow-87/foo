/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_LIBMAIN_H
#define ARCH_LIBMAIN_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define lib_crt0()				(arch_common_call(lib_crt0, -E_NOIMP)())


#endif // ARCH_LIBMAIN_H
