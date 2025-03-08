/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <sys/list.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <attr.h>
#include <nodes.h>
#include <parser.tab.h>


/* local/static prototypes */
static int index_add(node_t *node);
static node_t *index_query(char const *name);


/* static variables */
static vector_t node_index = VECTOR_INITIALISER(sizeof(node_t*));
static node_t *memory_nodes,
			  *device_nodes;


/* global functions */
int nodes_init(void){
	int r = 0;
	vector_t attrs = VECTOR_INITIALISER(sizeof(attr_t));


	if(attr_enlist(&attrs, attr_init(&(attr_t){}, "compatible", AT_STRING, 0, &ATTR_VALUE(p, ""))) != 0)
		return -1;

	r |= type_add("memory_root", TC_MEMORY, &VECTOR_INITIALISER(sizeof(attr_t)), 0x0);
	r |= type_add("devices_root", TC_DEVICE, &attrs, 0x0);

	if(r != 0)
		return -1;

	memory_nodes = node_create("memory_root", type_lookup("memory_root"), 0x0);
	device_nodes = node_create("device_root", type_lookup("devices_root"), 0x0);

	if(memory_nodes == 0x0 || device_nodes == 0x0)
		return -1;

	return attr_enlist(&device_nodes->attrs, attr_init(&(attr_t){}, "compatible", AT_STRING, 0, &ATTR_VALUE(p, "")));
}

node_t *nodes_root(type_cat_t category){
	switch(category){
	case TC_MEMORY:	return memory_nodes;
	case TC_DEVICE:	return device_nodes;
	default:		return 0x0;
	}
}

node_t *node_create(char const *name, type_t *type, node_t *childs){
	node_t *node,
		   *child;


	node = calloc(1, sizeof(node_t));

	if(node == 0x0)
		goto err_0;

	if(vector_init(&node->attrs, sizeof(attr_t), type->attrs.size) != 0)
		goto err_1;

	node->name = name;
	node->type = type;

	list_for_each(childs, child)
		node_child_add(node, child);

	if(index_add(node) != 0)
		goto err_1;

	return node;


err_1:
	free(node);

err_0:
	devtree_parser_error("%s: node allocation failed", name);

	return 0x0;
}

void node_destroy(node_t *node){
	vector_destroy(&node->attrs);
	free((char*)node->name);
	free(node);
}

int node_child_add(node_t *parent, node_t *child){
	if(parent != nodes_root(parent->type->category) && parent->type->category != child->type->category){
		return devtree_parser_error("%s: %s invalid child node type, expecting %s",
			parent->name,
			child->name,
			type_strcat(parent->type->category)
		);
	}

	child->parent = parent;
	list_add_tail(parent->childs, child);

	return 0;
}

node_t *node_ref(char const *name){
	node_t *node;


	node = index_query(name);

	if(node == 0x0)
		devtree_parser_error("%s: undefined reference", name);

	return node;
}


/* local functions */
static int index_add(node_t *node){
	if(index_query(node->name) != 0x0)
		return devtree_parser_error("%s: node already defined", node->name);

	return vector_add(&node_index, &node);
}

static node_t *index_query(char const *name){
	node_t **node;


	vector_for_each(&node_index, node){
		if(strcmp((*node)->name, name) == 0)
			return *node;
	}

	return 0x0;
}
