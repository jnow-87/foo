/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <stdlib.h>
#include <string.h>
#include <attr.h>
#include <nodes.h>
#include <parser.tab.h>


/* macros */
#define ATTR_ASSERT_MISSING(node, attr){ \
	if(node_attr_get(node, attr, 0) == 0x0) \
		return devtree_parser_error("%s: missing attribute \"%s\"", (node)->name, attr_name(attr)); \
}

#define ATTR_ASSERT_POW2(node, attr){ \
	unsigned long int _v = node_attr_get(node, attr, 0)->i; \
	if(bits_count(_v) != 1) \
		return devtree_parser_error("%s: attribute \"%s\" (%u) not a power of 2", (node)->name, attr_name(attr), _v); \
}


/* types */
typedef struct{
	char const *name;
	node_t *node;
} index_t;


/* local/static prototypes */
static int index_add(node_t *node);
static node_t *index_query(char const *name, size_t len);


/* static variables */
static node_t root_device,
				   root_memory,
				   root_arch;
static vector_t node_index;


/* global functions */
int nodes_init(void){
	if(vector_init(&node_index, sizeof(index_t), 16) != 0)
		return -1;

	memset(&root_device, 0, sizeof(node_t));
	root_device.type = NT_DEVICE;
	root_device.name = "device_root";

	if(vector_init(&root_device.attrs, sizeof(attr_t), 16) != 0)
		return -1;

	node_attr_add(&root_device, MT_COMPATIBLE, (attr_value_t){ .p = "" });

	memset(&root_memory, 0, sizeof(node_t));
	root_memory.type = NT_MEMORY;
	root_memory.name = "memory_root";

	if(vector_init(&root_memory.attrs, sizeof(attr_t), 16) != 0)
		return -1;

	node_attr_add(&root_memory, MT_BASE_ADDR, (attr_value_t){ .i = 0x0 });
	node_attr_add(&root_memory, MT_SIZE, (attr_value_t){ .i = 0 });

	memset(&root_arch, 0, sizeof(node_t));
	root_arch.type = NT_ARCH;
	root_arch.name = "arch_root";

	return vector_init(&root_arch.attrs, sizeof(attr_t), 16);
}

void *node_create(node_type_t type){
	node_t *node;


	node = calloc(1, sizeof(node_t));

	if(node == 0x0)
		goto err_0;

	if(vector_init(&node->attrs, sizeof(attr_t), 16) != 0)
		goto err_1;

	node->type = type;

	return node;


err_1:
	free(node);

err_0:
	devtree_parser_error("node allocation failed");

	return 0x0;
}

int node_child_add(node_t *parent, node_t *child){
	if(index_add(child))
		return -1;

	child->parent = parent;
	list_add_tail(parent->childs, child);

	return 0;
}

void node_assert_add(node_t *node, assert_t *a){
	list_add_tail(node->asserts, a);
}

int node_attr_add(node_t *node, attr_type_t type, attr_value_t value){
	attr_t a;


	a.type = type;
	a.value = value;

	if(vector_add(&node->attrs, &a) != 0)
		return devtree_parser_error("%s: adding attribute failed", node->name);

	return 0;
}

int node_attr_set(node_t *node, attr_type_t type, size_t idx, attr_value_t value){
	attr_value_t *v;


	v = node_attr_get(node, type, idx);

	if(v == 0x0)
		return devtree_parser_error("%s: missing attribute \"%s\"", node->name, attr_name(type));

	*v = value;

	return 0;
}

attr_value_t *node_attr_get(node_t *node, attr_type_t type, size_t idx){
	size_t i = 0;
	attr_t *a;


	vector_for_each(&node->attrs, a){
		if(a->type == type && i++ == idx)
			return &a->value;
	}

	return 0x0;
}

node_t *node_ref(char const *name, size_t len, node_type_t type){
	node_t *node;


	node = index_query(name, len);

	if(node != 0x0 && node->type == type)
		return node;

	if(node == 0x0)	devtree_parser_error("undefined reference \"%.*s\"", len, name);
	else			devtree_parser_error("invalid node type");

	return node;
}

attr_value_t *node_attr_ref(node_t *node, attr_type_t type, size_t idx){
	attr_value_t *v;


	v = node_attr_get(node, type, idx);

	if(v != 0x0)
		return v;

	devtree_parser_error("%s: undefined attribute \"%s\" or index (%zu) out of range", node->name, attr_name(type), idx);

	return 0x0;
}

unsigned long int *node_attr_ilist_ref(node_t *node, attr_type_t type, size_t idx){
	size_t i = 0;
	attr_t *a;
	unsigned long int *item;


	if(type != MT_INT_LIST && type != MT_REG_LIST){
		devtree_parser_error("%s: internal parser error: non-list type used on list", node->name);

		return 0x0;
	}

	vector_for_each(&node->attrs, a){
		if(a->type != type)
			continue;

		vector_for_each(a->value.lst->items, item){
			if(i++ == idx)
				return item;
		}
	}

	devtree_parser_error("%s: undefined attribute \"%s\" or index (%zu) out of range", node->name, attr_name(type), idx);

	return 0x0;
}

node_t *device_root(void){
	return &root_device;
}

node_t *memory_root(void){
	return &root_memory;
}

node_t *arch_root(void){
	return &root_arch;
}

int device_validate(node_t *node){
	ATTR_ASSERT_MISSING(node, MT_COMPATIBLE);

	if(*((char*)(node_attr_get(node, MT_COMPATIBLE, 0)->p)) == 0x0)
		return devtree_parser_error("%s: attribute \"compatible\" empty", node->name);

	return 0;
}

int memory_validate(node_t *node){
	ATTR_ASSERT_MISSING(node, MT_BASE_ADDR);
	ATTR_ASSERT_MISSING(node, MT_SIZE);

	if(node_attr_get(node, MT_SIZE, 0)->i == 0 && node->childs == 0x0)
		return devtree_parser_error("%s: zero-size memory", node->name);

	return 0;
}

int arch_validate(void){
	unsigned int ncores;


	ATTR_ASSERT_MISSING(&root_arch, MT_NCORES);
	ncores = node_attr_get(&root_arch, MT_NCORES, 0)->i;

	node_attr_add(&root_arch, MT_CORE_MASK, ATTR_VALUE(i, (0x1 << ncores) - 1));

	ATTR_ASSERT_MISSING(&root_arch, MT_ADDR_WIDTH);
	ATTR_ASSERT_MISSING(&root_arch, MT_REG_WIDTH);
	ATTR_ASSERT_MISSING(&root_arch, MT_NUM_INTS);
	ATTR_ASSERT_MISSING(&root_arch, MT_TIMER_INT);
	ATTR_ASSERT_MISSING(&root_arch, MT_SYSCALL_INT);
	ATTR_ASSERT_MISSING(&root_arch, MT_TIMER_CYCLE_TIME_US);

	if(ncores > 1)
		ATTR_ASSERT_MISSING(&root_arch, MT_IPI_INT);

	ATTR_ASSERT_POW2(&root_arch, MT_ADDR_WIDTH);
	ATTR_ASSERT_POW2(&root_arch, MT_REG_WIDTH);

	return 0;
}

void memory_node_complement(node_t *node){
	unsigned long int min = -1,
					  max = 0;
	unsigned long int base,
					  size;
	node_t *child;


	if(node == 0x0 || list_empty(node->childs))
		return;

	list_for_each(node->childs, child){
		memory_node_complement(child);

		base = node_attr_get(child, MT_BASE_ADDR, 0)->i;
		size = node_attr_get(child, MT_SIZE, 0)->i;

		if(min > base)
			min = base;

		if(max < base + size)
			max = base + size;
	}

	base = node_attr_get(node, MT_BASE_ADDR, 0)->i;
	size = node_attr_get(node, MT_SIZE, 0)->i;

	if(size != 0 && base != min)
		devtree_parser_error("%s: memory base doesn't match childs, should be %#x\n", node->name, min);

	if(size != 0 && size != max - min)
		devtree_parser_error("%s: memory size doesn't match childs, should be %u\n", node->name, max - min);

	node_attr_set(node, MT_BASE_ADDR, 0, ATTR_VALUE(i, min));
	node_attr_set(node, MT_SIZE, 0, ATTR_VALUE(i, max - min));
}


/* local functions */
static int index_add(node_t *node){
	index_t *i;


	vector_for_each(&node_index, i){
		if(strcmp(i->name, node->name) == 0)
			return devtree_parser_error("node \"%s\" already defined", node->name);
	}

	return vector_add(&node_index, &(index_t){ .name = node->name, .node = node });
}

static node_t *index_query(char const *name, size_t len){
	index_t *i;


	vector_for_each(&node_index, i){
		if(strncmp(i->name, name, len) == 0 && strlen(i->name) == len)
			return i->node;
	}

	return 0x0;
}
