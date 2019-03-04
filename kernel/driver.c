/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/devtree.h>
#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <sys/string.h>


/* external variables */
extern driver_interface_t __driver_interface_base[],
						  __driver_interface_end[];
extern driver_device_t __driver_device_base[],
					   __driver_device_end[];

extern devtree_t __dt_root;


/* local/static prototypes */
static void probe_childs(devtree_t const * const *childs, void *itf);
static void probe_interface(devtree_t const *node, void *itf);
static void probe_device(devtree_t const *node, void *itf);


/* global functions */
void driver_load(void){
	INFO("load drivers\n");
	probe_childs(__dt_root.childs, 0x0);
}


/* local functions */
static void probe_childs(devtree_t const * const *childs, void *itf){
	size_t i;


	for(i=0; childs[i]!=0x0; i++){
		if(childs[i]->childs != 0x0)	probe_interface(childs[i], itf);
		else							probe_device(childs[i], itf);
	}
}

static void probe_interface(devtree_t const *node, void *itf){
	driver_interface_t *e;


	for(e=__driver_interface_base; e!=__driver_interface_end; e++){
		if(strcmp(e->compatible, node->compatible) == 0){
			INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

			itf = e->probe((void*)node->data, itf);

			if(itf == 0x0){
				INFO("failed\n");
				return;
			}

			probe_childs(node->childs, itf);
			return;
		}
	}

	INFO("no compatible interface driver found for \"%s\"\n", node->name);
}

static void probe_device(devtree_t const *node, void *itf){
	driver_device_t *e;


	for(e=__driver_device_base; e!=__driver_device_end; e++){
		if(strcmp(e->compatible, node->compatible) == 0){
			INFO("probe driver \"%s\" for \"%s\"\n", e->compatible, node->name);

			if(e->probe(node->name, (void*)node->data, itf) != 0)
				INFO("failed\n");

			return;
		}
	}

	INFO("no compatible device driver found for \"%s\"\n", node->name);
}
