/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devtree.h>
#include <sys/string.h>


/* macros */
#define FIND_NODE_BY_STRING(root, member, str, rec_func_name) \
	unsigned int i; \
	typeof(root) node; \
	\
	\
	if(strcmp(root->member, str) == 0) \
		return root; \
	\
	if(root->childs == 0x0) \
		return 0x0; \
	\
	for(i=0; root->childs[i]!=0x0; i++){ \
		if(strcmp(root->childs[i]->member, str) == 0) \
			return root->childs[i]; \
	} \
	\
	for(i=0; root->childs[i]!=0x0; i++){ \
		node = rec_func_name(root->childs[i], str); \
		\
		if(node != 0x0) \
			return node; \
	} \
	\
	return 0x0;


/* global functions */
devtree_device_t const *devtree_find_device_by_name(devtree_device_t const *root, char const *name){
	FIND_NODE_BY_STRING(root, name, name, devtree_find_device_by_name);
}

devtree_device_t const *devtree_find_device_by_comp(devtree_device_t const *root, char const *comp){
	FIND_NODE_BY_STRING(root, compatible, comp, devtree_find_device_by_comp);
}

devtree_memory_t const *devtree_find_memory_by_name(devtree_memory_t const *root, char const *name){
	FIND_NODE_BY_STRING(root, name, name, devtree_find_memory_by_name);
}
