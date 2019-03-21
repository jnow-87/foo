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
typedef struct devtree_t{
	char const *name,
			   *compatible;

	void const *data;

	struct devtree_t const *parent;
	struct devtree_t const * const *childs;
} devtree_t;


#endif // KERNEL_DEVTREE_H
