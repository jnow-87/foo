/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/register.h>
#include <sys/vector.h>
#include <sys/list.h>
#include <string.h>
#include <stdlib.h>
#include <parser.tab.h>
#include <nodes.h>


/* macros */
#define ARCH_ASSERT_MISSING(member, default){ \
	if(root_arch.member == default) \
		return devtree_parser_error("missing arch attribute '" #member "'"); \
}

#define ARCH_ASSERT_POW2(member){ \
	if(bits_set(root_arch.member) != 1) \
		return devtree_parser_error("arch attribute '" #member "' not a power of 2"); \
}


/* local/static prototypes */
static void *alloc_node(size_t size, node_type_t type);
static int add_name(char const *name);

static int validate_device(device_node_t *node);
static int validate_memory(memory_node_t *node);


/* static variables */
static device_node_t root_device;
static memory_node_t root_memory;
static arch_node_t root_arch;
static vector_t node_names;


/* global functions */
int nodes_init(void){
	memset(&root_device, 0, sizeof(device_node_t));
	root_device.type = NT_DEVICE;
	root_device.name = "device_root";
	root_device.compatible = "";

	memset(&root_memory, 0, sizeof(memory_node_t));
	root_memory.type = NT_MEMORY;
	root_memory.name = "memory_root";

	memset(&root_arch, 0, sizeof(arch_node_t));
	root_arch.type = NT_ARCH;
	root_arch.name = "arch_root";
	root_arch.addr_width = 0;
	root_arch.reg_width = 0;
	root_arch.core_mask = 0x0;
	root_arch.ncores = 0;
	root_arch.num_ints = -1;
	root_arch.num_vints = -1;
	root_arch.timer_int = -1;
	root_arch.syscall_int = -1;
	root_arch.ipi_int = -1;
	root_arch.timer_cycle_time_us = 0;

	return vector_init(&node_names, sizeof(char*), 16);
}

device_node_t *device_root(void){
	return &root_device;
}

device_node_t *device_node_alloc(void){
	device_node_t *node;


	node = alloc_node(sizeof(device_node_t), NT_DEVICE);

	if(node == 0x0)
		goto err_0;

	if(vector_init(&node->payload, sizeof(member_t), 16) != 0)
		goto err_1;

	return node;


err_1:
	free(node);

err_0:
	return 0x0;
}

int device_node_add_member(device_node_t *node, member_type_t type, void *payload){
	member_t m;


	m.type = type;
	m.payload = payload;

	if(vector_add(&node->payload, &m) != 0)
		return devtree_parser_error("adding member failed");

	return 0;
}

memory_node_t *memory_root(void){
	return &root_memory;
}

memory_node_t *memory_node_alloc(void){
	return alloc_node(sizeof(memory_node_t), NT_MEMORY);
}

void memory_node_complement(memory_node_t *node){
	void *min = (void*)0xffffffff,
		 *max = 0x0;
	memory_node_t *child;


	if(node == 0x0 || list_empty(node->childs))
		return;

	list_for_each(node->childs, child){
		memory_node_complement(child);

		if(min > child->base)
			min = child->base;

		if(max < child->base + child->size)
			max = child->base + child->size;
	}

	if(node->size == 0){
		node->base = min;
		node->size = max - min;
	}
}

arch_node_t *arch_root(void){
	return &root_arch;
}

int arch_validate(void){
	root_arch.core_mask = (0x1 << root_arch.ncores) - 1;

	ARCH_ASSERT_MISSING(addr_width, 0);
	ARCH_ASSERT_MISSING(reg_width, 0);
	ARCH_ASSERT_MISSING(ncores, 0);
	ARCH_ASSERT_MISSING(num_ints, -1);
	ARCH_ASSERT_MISSING(num_vints, -1);
	ARCH_ASSERT_MISSING(timer_int, -1);
	ARCH_ASSERT_MISSING(syscall_int, -1);
	ARCH_ASSERT_MISSING(timer_cycle_time_us, 0);

	if(root_arch.ncores > 1)
		ARCH_ASSERT_MISSING(ipi_int, -1);

	ARCH_ASSERT_POW2(addr_width);
	ARCH_ASSERT_POW2(reg_width);

	return 0;
}

int node_add_child(base_node_t *parent, base_node_t *child){
	int r;


	switch(child->type){
	case NT_DEVICE:	r = validate_device((device_node_t*)child); break;
	case NT_MEMORY:	r = validate_memory((memory_node_t*)child); break;
	default:		r = devtree_parser_error("invalid node type %d", child->type);
	}

	if(r != 0 || add_name(child->name))
		return -1;

	child->parent = parent;
	list_add_tail(parent->childs, child);

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
static void *alloc_node(size_t size, node_type_t type){
	base_node_t *node;


	node = calloc(1, size);

	if(node == 0x0)
		goto err;

	node->type = type;

	return node;


err:
	devtree_parser_error("node allocation failed");

	return 0x0;
}

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

	return 0;
}

static int validate_memory(memory_node_t *node){
	if(node->size == 0 && node->childs == 0x0)
		return devtree_parser_error("zero-size memory node \"%s\"", node->name);

	return 0;
}
