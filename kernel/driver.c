/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <sys/devtree.h>
#include <sys/string.h>


/* external variables */
extern interface_driver_t __interface_driver_base[],
						  __interface_driver_end[];
extern device_driver_t __device_driver_base[],
					   __device_driver_end[];


/* local/static prototypes */
static void probe_childs(devtree_device_t const * const *childs, void *itf);
static void probe_interface(devtree_device_t const *node, void *itf);
static void probe_device(devtree_device_t const *node, void *itf);


/* global functions */
int driver_load(void){
	INFO("load drivers\n");
	probe_childs(__dt_devices_root.childs, 0x0);

	return -errno;
}


/* local functions */
static void probe_childs(devtree_device_t const * const *childs, void *itf){
	size_t i;


	if(childs == 0x0)
		return;

	for(i=0; childs[i]!=0x0; i++){
		if(childs[i]->childs != 0x0)	probe_interface(childs[i], itf);
		else							probe_device(childs[i], itf);
	}
}

static void probe_interface(devtree_device_t const *node, void *itf){
	interface_driver_t *e;


	for(e=__interface_driver_base; e!=__interface_driver_end; e++){
		if(strcmp(e->compatible, node->compatible) == 0){
			INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

			itf = e->probe(node->name, (void*)node->data, itf);

			if(itf)	probe_childs(node->childs, itf);
			else	INFO("probe error \"%s\"\n", strerror(errno));

			return;
		}
	}

	INFO("no compatible interface driver found for \"%s\"\n", node->name);
}

static void probe_device(devtree_device_t const *node, void *itf){
	device_driver_t *e;


	for(e=__device_driver_base; e!=__device_driver_end; e++){
		if(strcmp(e->compatible, node->compatible) == 0){
			INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

			if(e->probe(node->name, (void*)node->data, itf) != 0)
				INFO("failed\n");

			return;
		}
	}

	INFO("no compatible device driver found for \"%s\"\n", node->name);
}
