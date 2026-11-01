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
#include "attr.h"
#include "expr.h"


/* macros */
#define NODE_ATTRVEC_INITIALISER()	(node_attrvec_t){ .v = VECTOR_INITIALISER(sizeof(node_attr_t)) }


/* incomplete types */
struct type_t;


/* types */
typedef struct{
	char const *name;
	expr_value_t value;
} node_attr_t;

ENSURE_ATTR_TYPE(node_attr_t);

typedef struct node_attrvec_t{
	attrvec_t v;
} node_attrvec_t;

typedef struct node_t{
	struct node_t *prev,
				  *next,
				  *childs;

	char const *name;
	struct type_t *type;
	node_attrvec_t attrs;
} node_t;


/* prototypes */
void nodes_free(void);
node_t *nodes_root();

node_t *node_alloc(char const *name, struct type_t *type, node_t *childs);
void node_free(node_t *node);

node_t *node_ref(char const *name);

node_attr_t *node_attr_init(node_attr_t *attr, char const *name, expr_value_t *value);
int node_attr_update(node_t *node, node_attr_t *attr, expr_op_t op, expr_t *arg);


#endif // DEVTREE_NODE_H
