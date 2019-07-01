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
} member_type_t;

typedef struct{
	size_t size;
	vector_t *lst;
} member_int_t;

typedef struct{
	member_type_t type;
	void *data;
} member_t;

typedef struct driver_node_t{
	struct driver_node_t *prev,
						 *next,
						 *parent,
						 *childs;

	char const *name,
			   *compatible;

	vector_t data;			/**< vector of member_t elements */
} driver_node_t;

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
int node_export_driver(driver_node_t *node, FILE *fp);
int node_export_memory(memory_node_t *node, FILE *fp);


#endif // DEVTREE_NODE_H
