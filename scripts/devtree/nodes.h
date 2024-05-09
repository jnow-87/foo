/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_NODE_H
#define DEVTREE_NODE_H


#include <sys/types.h>
#include <sys/vector.h>
#include <asserts.h>
#include <attr.h>


/* types */
typedef enum{
	NT_DEVICE = 1,
	NT_MEMORY,
	NT_ARCH,
} node_type_t;

typedef struct node_t{
	struct node_t *prev,
				  *next,
				  *parent,
				  *childs;

	node_type_t type;
	char const *name;
	assert_t *asserts;
	vector_t attrs;
} node_t;


/* prototypes */
int nodes_init(void);
void *node_create(node_type_t type);

int node_child_add(node_t *parent, node_t *child);
void node_assert_add(node_t *node, assert_t *a);

int node_attr_add(node_t *node, attr_type_t type, attr_value_t value);
int node_attr_set(node_t *node, attr_type_t type, size_t idx, attr_value_t value);
attr_value_t *node_attr_get(node_t *node, attr_type_t type, size_t idx);

node_t *device_root(void);
node_t *memory_root(void);
node_t *arch_root(void);

int device_validate(node_t *node);
int memory_validate(node_t *node);
int arch_validate(void);

void memory_node_complement(node_t *node);


#endif // DEVTREE_NODE_H
