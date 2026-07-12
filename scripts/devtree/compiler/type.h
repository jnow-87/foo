/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_TYPES_H
#define DEVTREE_TYPES_H


#include <stdbool.h>
#include <sys/vector.h>
#include "assert.h"
#include "attr.h"


/* incomplete types */
struct node_t;


/* types */
typedef struct type_t{
	struct type_t *prev,
				  *next;

	char const *name;
	assert_t *asserts;
	attr_vec_t attrs;
} type_t;


/* prototypes */
int type_create(char const *name, attr_vec_t *attrs, assert_t *asserts);
type_t *type_lookup(char const *name);
struct node_t *type_instantiate(type_t *type, char const *name, attr_vec_t *attrs, struct node_t *childs);


#endif // DEVTREE_TYPES_H
