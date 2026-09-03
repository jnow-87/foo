/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_NODE_H
#define DEVTREE_NODE_H


#include <sys/types.h>
#include "assert.h"
#include "attrvec.h"
#include "type.h"


/* types */
typedef struct node_t{
	struct node_t *prev,
				  *next,
				  *childs;

	char const *name;
	type_t *type;
	attrvec_t attrs;
} node_t;


/* prototypes */
int nodes_init(void);
node_t *nodes_root();
int nodes_assert(void);

node_t *node_create(char const *name, type_t *type, node_t *childs);
void node_destroy(node_t *node);

node_t *node_ref(char const *name);


#endif // DEVTREE_NODE_H
