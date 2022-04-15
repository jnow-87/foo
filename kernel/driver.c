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
	if(errno){
		FATAL("pre-driver-load error %s\n", strerror(errno));
		return -errno;
	}

	INFO("load drivers\n");

	return probe_childs(__dt_devices_root.childs, 0x0);
}


/* local functions */
static int probe_childs(devtree_device_t const * const *childs, void *itf){
	size_t i;


	if(childs == 0x0){
		if(itf && iskheap(itf))
			kfree(itf);

		return E_OK;
	}

	for(i=0; childs[i]!=0x0; i++){
		if(probe(childs[i], itf) != E_OK){
			FATAL("driver probe error for \"%s\": %s\n", childs[i]->name, strerror(errno));
			return -errno;
		}
	}

	return E_OK;
}

static int probe(devtree_device_t const *node, void *itf){
	driver_t *e;


	for(e=__driver_base; e!=__driver_end; e++){
		if(strcmp(e->compatible, node->compatible) != 0)
			continue;

		INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

		itf = e->probe(node->name, (void*)node->data, itf);

		if(errno)
			return -errno;

		if(itf != 0x0)
			return probe_childs(node->childs, itf);

		if(node->childs == 0x0)
			return E_OK;

		FATAL("no driver interface for \"%s\" childs\n", node->name);

		return_errno(E_UNAVAIL);
	}

	INFO("no compatible driver found for \"%s\"\n", node->name);

	return E_OK;
}
