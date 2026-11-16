/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifdef BUILD_KERNEL
# include <kernel/panic.h>
#endif // BUILD_KERNEL

#include <sys/devtree.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/types.h>


/* local/static prototypes */
static devtree_node_t const *find(devtree_node_t const *node, char const *comp, char const *name);
static void traverse(devtree_node_t const *node, devtree_action_t action, devtree_filter_t filter, size_t level, void *data);


/* global functions */
void const *devtree_find(char const *comp, char const *name){
	devtree_node_t const *node;


	node = find(&__dt_root, comp, name);

	if(node == 0x0 || node->payload == 0x0){
#ifdef BUILD_KERNEL
		kpanic("no matching devtree node for name=\"%\", comp=\"%s\"\n", name, comp);
#endif // BUILD_KERNEL
		goto_errno(err, E_INVAL);
	}

	return node->payload;


err:
	return 0x0;
}

void devtree_traverse(devtree_action_t action, devtree_filter_t filter, void *data){
	traverse(&__dt_root, action, filter, 0, data);
}

int devtree_filter_device(devtree_node_t const *node){
	return (strcmp(node->compatible, "memory") != 0);
}

int devtree_filter_memory(devtree_node_t const *node){
	return strcmp(node->compatible, "memory");
}


/* local functions */
static devtree_node_t const *find(devtree_node_t const *node, char const *comp, char const *name){
	if((name == 0x0 || strcmp(node->name, name) == 0) && (comp == 0x0 || strcmp(node->compatible, comp) == 0))
		return node;

	for(devtree_node_t const * const *child=node->childs; child!=0x0 && *child!=0x0; child++){
		node = find(*child, comp, name);

		if(node != 0x0)
			return node;
	}

	return 0x0;
}

static void traverse(devtree_node_t const *node, devtree_action_t action, devtree_filter_t filter, size_t level, void *data){
	if((filter == 0x0 || filter(node) == 0) && action(node, level, data) != 0)
		return;

	for(devtree_node_t const * const *child=node->childs; child!=0x0 && *child!=0x0; child++)
		traverse(*child, action, filter, level + 1, data);
}
