/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_DEVTREE_H
#define SYS_DEVTREE_H


#ifndef BUILD_HOST
#include <sys/types.h>
#else
#include <stdint.h>
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
	uint32_t size;

	struct devtree_memory_t const *parent;
	struct devtree_memory_t const * const *childs;
} devtree_memory_t;


/* prototypes */
devtree_driver_t const *devtree_find_driver_by_name(devtree_driver_t const *root, char const *name);
devtree_driver_t const *devtree_find_driver_by_comp(devtree_driver_t const *root, char const *name);

devtree_memory_t const *devtree_find_memory_by_name(devtree_memory_t const *root, char const *name);


#endif // SYS_DEVTREE_H
