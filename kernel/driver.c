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
static void probe_childs(devtree_device_t const * const *childs, void *itf);
static void probe(devtree_device_t const *node, void *itf);
static driver_t *match_driver(devtree_device_t const *node);


/* global functions */
void driver_load(void){
	INFO("load drivers\n");
	probe_childs(__dt_device_root.childs, 0x0);
}


/* local functions */
static void probe_childs(devtree_device_t const * const *childs, void *itf){
	if(childs == 0x0)
		return;

	for(size_t i=0; childs[i]!=0x0; i++){
		probe(childs[i], itf);
	}
}

static void probe(devtree_device_t const *node, void *itf){
	driver_t *drv;


	drv = match_driver(node);

	if(drv == 0x0)
		return;

	INFO("probe driver \"%s\" for \"%s\"\n", drv->compatible, node->name);
	itf = drv->probe(node->name, (void*)node->payload, itf);

	if(errno){
		WARN("driver probe error for \"%s\": %s\n", node->name, strerror(errno));
		goto err_1;
	}

	if(node->childs == 0x0 && itf != 0x0)
		goto err_1;

	if(node->childs != 0x0 && itf == 0x0){
		WARN("no driver interface for \"%s\" childs\n", node->name);
		goto err_0;
	}

	probe_childs(node->childs, itf);

	return;


err_1:
	if(itf && iskheap(itf))
		kfree(itf);

err_0:
	reset_errno();
}

static driver_t *match_driver(devtree_device_t const *node){
	for(driver_t *e=__driver_base; e!=__driver_end; e++){
		if(strcmp(e->compatible, node->compatible) == 0)
			return e;
	}

	INFO("no compatible driver found for \"%s\"\n", node->name);

	return 0x0;
}
