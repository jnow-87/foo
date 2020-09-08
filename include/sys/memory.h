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
#else
#ifdef BUILD_HOST
#include <stdlib.h>
#else
#include <lib/stdlib.h>
#endif // BUILD_HOST
#endif // BUILD_KERNEL


/* macros */
#ifdef BUILD_KERNEL
#define	sys_malloc	kmalloc
#define	sys_calloc	kcalloc
#define sys_free	kfree
#else
#ifdef BUILD_HOST
#define sys_malloc	mallocp
#define sys_calloc	callocp
#define sys_free	freep
#else
#define sys_malloc	malloc
#define sys_calloc	calloc
#define sys_free	free
#endif // BUILD_HOST
#endif // BUILD_KERNEL


/* global variables */
#ifdef BUILD_HOST
extern void *(*mallocp)(size_t size);
extern void (*freep)(void *addr);
extern void *(*callocp)(size_t n, size_t size);
#endif // BUILD_HOST


#endif // SYS_MEMORY_H
