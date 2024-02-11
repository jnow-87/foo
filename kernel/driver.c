/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <kernel/memory.h>
#include <sys/devtree.h>
#include <sys/string.h>
#include <sys/errno.h>


/* external variables */
extern driver_t __driver_base[],
				__driver_end[];


/* local/static prototypes */
static int probe_childs(devtree_device_t const * const *childs, void *itf);
static int probe(devtree_device_t const *node, void *itf);


/* global functions */
int driver_load(void){
	INFO("load drivers\n");

	return probe_childs(__dt_device_root.childs, 0x0);
}


/* local functions */
static int probe_childs(devtree_device_t const * const *childs, void *itf){
	if(childs == 0x0){
		if(itf && iskheap(itf))
			kfree(itf);

		return 0;
	}

	for(size_t i=0; childs[i]!=0x0; i++){
		if(probe(childs[i], itf) != 0){
			FATAL("driver probe error for \"%s\": %s\n", childs[i]->name, strerror(errno));
			return -errno;
		}
	}

	return 0;
}

static int probe(devtree_device_t const *node, void *itf){
	for(driver_t *e=__driver_base; e!=__driver_end; e++){
		if(strcmp(e->compatible, node->compatible) != 0)
			continue;

		INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

		itf = e->probe(node->name, (void*)node->payload, itf);

		if(errno)
			return -errno;

		if(itf != 0x0)
			return probe_childs(node->childs, itf);

		if(node->childs == 0x0)
			return 0;

		FATAL("no driver interface for \"%s\" childs\n", node->name);

		return_errno(E_UNAVAIL);
	}

	INFO("no compatible driver found for \"%s\"\n", node->name);

	return 0;
}
