/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/devtree.h>
#include <sys/string.h>


/* types */
typedef void const * (*traverse_t)(void const *root, char const *ref);


/* local/static prototypes */
static void const *find(void const *root, void const *childs, size_t key_offset, char const *ref, traverse_t traverse);


/* global functions */
devtree_device_t const *devtree_find_device_by_name(devtree_device_t const *root, char const *name){
	return find(root, root->childs, offsetof(devtree_device_t, name), name, (traverse_t)devtree_find_device_by_name);
}

devtree_device_t const *devtree_find_device_by_comp(devtree_device_t const *root, char const *comp){
	return find(root, root->childs, offsetof(devtree_device_t, compatible), comp, (traverse_t)devtree_find_device_by_comp);
}

devtree_memory_t const *devtree_find_memory_by_name(devtree_memory_t const *root, char const *name){
	return find(root, root->childs, offsetof(devtree_memory_t, name), name, (traverse_t)devtree_find_memory_by_name);
}


/* local functions */
static void const *find(void const *root, void const *_childs, size_t key_offset, char const *ref, traverse_t traverse){
	char **childs = (char**)_childs;
	void const *child;


	if(strcmp(*((char**)((char*)root + key_offset)), ref) == 0)
		return root;

	if(childs == 0x0)
		return 0x0;

	for(size_t i=0; childs[i]!=0x0; i++){
		if(strcmp(childs[i] + key_offset, ref) == 0)
			return childs[i];
	}

	for(size_t i=0; childs[i]!=0x0; i++){
		child = traverse(childs[i], ref);

		if(child != 0x0)
			return child;
	}

	return 0x0;
}
