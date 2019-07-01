/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_DEVTREE_H
#define KERNEL_DEVTREE_H


#ifndef BUILD_HOST
#include <sys/types.h>
#else
#include <stdio.h>
#endif // BUILD_HOST


/* types */
typedef struct devtree_driver_t{
	char const *name,
			   *compatible;

	void const *data;

	struct devtree_driver_t const *parent;
	struct devtree_driver_t const * const *childs;
} devtree_driver_t;

typedef struct devtree_memory_t{
	char const *name;
	void *base;
	size_t size;

	struct devtree_memory_t const *parent;
	struct devtree_memory_t const * const *childs;
} devtree_memory_t;


#endif // KERNEL_DEVTREE_H
