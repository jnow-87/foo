/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/vector.h>
#include <sys/list.h>
#include <string.h>
#include <stdlib.h>
#include <parser.tab.h>
#include <nodes.h>


/* local/static prototypes */
static int add_name(char const *name);

static int validate_device(device_node_t *node);
static int validate_memory(memory_node_t *node);


/* static variables */
static device_node_t root_device;
static memory_node_t root_memory;
static vector_t node_names;


/* global functions */
int nodes_init(void){
	memset(&root_device, 0, sizeof(device_node_t));
	root_device.name = "device_root";
	root_device.compatible = "";

	memset(&root_memory, 0, sizeof(memory_node_t));
	root_memory.name = "memory_root";

	return vector_init(&node_names, sizeof(char*), 16);
}

void nodes_cleanup(void){
	vector_destroy(&node_names);
}

device_node_t *device_root(void){
	return &root_device;
}

device_node_t *device_node_alloc(void){
	device_node_t *node;


	node = calloc(1, sizeof(device_node_t));

	if(node == 0x0)
		goto err_0;

	if(vector_init(&node->payload, sizeof(member_t), 16) != 0)
		goto err_1;

	return node;


err_1:
	free(node);

err_0:
	devtree_parser_error("device node allocation failed");

	return 0x0;
}

int device_node_add_child(device_node_t *parent, device_node_t *node){
	if(validate_device(node) != 0)
		return -1;

	node->parent = parent;
	list_add_tail(parent->childs, node);

	return 0;
}

int device_node_add_member(device_node_t *node, member_type_t type, void *payload){
	member_t m;


	m.type = type;
	m.payload = payload;

	if(vector_add(&node->payload, &m) != 0)
		return devtree_parser_error("adding member failed");

	return 0;
}

int device_node_set_compatible(device_node_t *node, char const *compatible){
	if(node->compatible != 0x0)
		return devtree_parser_error("attribute \"compatible\" already set");

	node->compatible = compatible;

	return 0;
}

memory_node_t *memory_root(void){
	return &root_memory;
}

memory_node_t *memory_node_alloc(void){
	memory_node_t *node;


	node = calloc(1, sizeof(memory_node_t));

	if(node == 0x0)
		devtree_parser_error("memory node allocation failed");

	return node;
}

int memory_node_add_child(memory_node_t *parent, memory_node_t *node){
	if(validate_memory(node) != 0)
		return -1;

	node->parent = parent;
	list_add_tail(parent->childs, node);

	return 0;
}

void *node_intlist_alloc(size_t size, void *payload){
	member_int_t *lst;


	lst = malloc(sizeof(member_int_t));

	if(lst == 0x0)
		goto err;

	lst->size = size;
	lst->lst = payload;

	return lst;


err:
	devtree_parser_error("intlist allocation failed");

	return 0x0;
}


/* local functions */
static int add_name(char const *name){
	char const **xname;


	vector_for_each(&node_names, xname){
		if(strcmp(*xname, name) == 0)
			return devtree_parser_error("node \"%s\" already defined", name);
	}

	return vector_add(&node_names, &name);
}

static int validate_device(device_node_t *node){
	if(node->compatible == 0x0 || *node->compatible == 0)
		return devtree_parser_error("attribute \"compatible\" not set");

	return add_name(node->name);
}

static int validate_memory(memory_node_t *node){
	if(node->size == 0 && node->childs == 0x0)
		return devtree_parser_error("zero-size memory node \"%s\"", node->name);

	return add_name(node->name);
}
