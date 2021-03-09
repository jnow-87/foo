/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_DEVTREE_H
#define SYS_DEVTREE_H


#ifndef BUILD_HOST
# include <sys/types.h>
#else
# include <stdint.h>
# include <stdio.h>
#endif // BUILD_HOST


/* types */
typedef struct devtree_device_t{
	char const *name,
			   *compatible;

	void const *data;

	struct devtree_device_t const *parent;
	struct devtree_device_t const * const *childs;
} devtree_device_t;

typedef struct devtree_memory_t{
	char const *name;
	void *base;
	uint32_t size;

	struct devtree_memory_t const *parent;
	struct devtree_memory_t const * const *childs;
} devtree_memory_t;


/* prototypes */
devtree_device_t const *devtree_find_device_by_name(devtree_device_t const *root, char const *name);
devtree_device_t const *devtree_find_device_by_comp(devtree_device_t const *root, char const *name);

devtree_memory_t const *devtree_find_memory_by_name(devtree_memory_t const *root, char const *name);


/* external variables */
extern devtree_device_t const __dt_devices_root;
extern devtree_memory_t const __dt_memory_root;


#endif // SYS_DEVTREE_H
