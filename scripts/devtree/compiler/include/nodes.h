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
#include <types.h>


/* types */
typedef struct node_t{
	struct node_t *prev,
				  *next,
				  *parent,
				  *childs;

	char const *name;
	type_t *type;
	vector_t attrs;
} node_t;


/* prototypes */
int nodes_init(void);
node_t *nodes_root(type_cat_t category);

node_t *node_create(char const *name, type_t *type, node_t *childs);
void node_destroy(node_t *node);

int node_child_add(node_t *parent, node_t *child);

node_t *node_ref(char const *name);


#endif // DEVTREE_NODE_H
