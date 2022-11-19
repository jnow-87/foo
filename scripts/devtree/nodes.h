/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_NODE_H
#define DEVTREE_NODE_H


#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <stdio.h>


/* macros */
#define BASE_NODE_COMPATIBLE(node_type) \
	STATIC_ASSERT(offsetof(node_type, prev) == offsetof(base_node_t, prev)); \
	STATIC_ASSERT(offsetof(node_type, next) == offsetof(base_node_t, next)); \
	STATIC_ASSERT(offsetof(node_type, parent) == offsetof(base_node_t, parent)); \
	STATIC_ASSERT(offsetof(node_type, childs) == offsetof(base_node_t, childs)); \
	STATIC_ASSERT(offsetof(node_type, type) == offsetof(base_node_t, type)); \
	STATIC_ASSERT(offsetof(node_type, name) == offsetof(base_node_t, name));


/* types */
typedef enum{
	NT_DEVICE = 1,
	NT_MEMORY,
	NT_ARCH,
} node_type_t;

typedef enum{
	MT_BASE_ADDR = 1,
	MT_REG_LIST,
	MT_INT_LIST,
	MT_STRING,
} member_type_t;

typedef struct{
	size_t size;
	vector_t *lst;
} member_int_t;

typedef struct{
	member_type_t type;
	void *payload;
} member_t;

typedef struct base_node_t{
	struct base_node_t *prev,
					   *next,
					   *parent,
					   *childs;

	node_type_t type;
	char const *name;
} base_node_t;

typedef struct device_node_t{
	struct device_node_t *prev,
						 *next,
						 *parent,
						 *childs;

	node_type_t type;
	char const *name,
			   *compatible;

	vector_t payload;			/**< vector of member_t elements */
} device_node_t;

BASE_NODE_COMPATIBLE(device_node_t)

typedef struct memory_node_t{
	struct memory_node_t *prev,
						 *next,
						 *parent,
						 *childs;

	node_type_t type;
	char const *name;
	void *base;
	size_t size;
} memory_node_t;

BASE_NODE_COMPATIBLE(memory_node_t)

typedef struct arch_node_t{
	struct arch_node_t *prev,
					   *next,
					   *parent;
	device_node_t *childs;

	node_type_t type;
	char const *name;

	int8_t num_ints,
		   num_vints;
} arch_node_t;

BASE_NODE_COMPATIBLE(arch_node_t)

/* prototypes */
int nodes_init(void);

device_node_t *device_root(void);
device_node_t *device_node_alloc(void);
int device_node_add_member(device_node_t *node, member_type_t type, void *payload);

memory_node_t *memory_root(void);
memory_node_t *memory_node_alloc(void);
void memory_node_complement(memory_node_t *node);

arch_node_t *arch_root(void);
int arch_validate(void);

int node_add_child(base_node_t *parent, base_node_t *child);
void *node_intlist_alloc(size_t size, void *payload);


#endif // DEVTREE_NODE_H
