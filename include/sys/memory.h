/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_MEMORY_H
#define SYS_MEMORY_H


#ifdef BUILD_KERNEL

#include <kernel/memory.h>

#define	sys_malloc	kmalloc
#define	sys_calloc	kcalloc
#define sys_free	kfree

#else

#ifdef BUILD_HOST
#include <stdlib.h>
#else
#include <lib/stdlib.h>
#endif // BUILD_HOST

#define sys_malloc	malloc
#define sys_calloc	calloc
#define sys_free	free

#endif // BUILD_KERNEL


#endif // SYS_MEMORY_H
