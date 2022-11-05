/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_NODE_H
#define DEVTREE_NODE_H


#include <sys/vector.h>
#include <stdio.h>


/* types */
typedef enum{
	NT_SEC_DRIVER = 1,
	NT_SEC_MEMORY,
	NT_DATA,
} node_type_t;

typedef enum{
	MT_BASE_ADDR = 1,
	MT_REG_LIST,
	MT_INT_LIST,
	MT_SIZE,
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

typedef struct device_node_t{
	struct device_node_t *prev,
						 *next,
						 *parent,
						 *childs;

	char const *name,
			   *compatible;

	vector_t payload;			/**< vector of member_t elements */
} device_node_t;

typedef struct memory_node_t{
	struct memory_node_t *prev,
						 *next,
						 *parent,
						 *childs;

	char const *name;
	void *base;
	size_t size;
} memory_node_t;


/* prototypes */
int nodes_init(void);
void nodes_cleanup(void);

device_node_t *device_root(void);
device_node_t *device_node_alloc(void);
int device_node_add_child(device_node_t *parent, device_node_t *node);
int device_node_add_member(device_node_t *node, member_type_t type, void *payload);
int device_node_set_compatible(device_node_t *node, char const *compatible);

memory_node_t *memory_root(void);
memory_node_t *memory_node_alloc(void);
int memory_node_add_child(memory_node_t *parent, memory_node_t *node);

void *node_intlist_alloc(size_t size, void *payload);


#endif // DEVTREE_NODE_H
