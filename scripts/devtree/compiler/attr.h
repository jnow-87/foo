/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ATTRS_H
#define DEVTREE_ATTRS_H


#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/vector.h>
#include "expr.h"



/**
 * TODO
 *   should there be two types of attributes, one for nodes one for types
 *   type attributes allow expressions
 *   node attributes allow only literal expression, i.e. expr_value_t
 *
 *   type and array_limit should then only be needed for type attributes
 */

/* macros */
#define ENSURE_ATTR_TYPE(type) \
	_Static_assert(offsetof(attr_t, name) == offsetofvar(type, name), \
		#type " is not compatible with attr_t, due to mismatching offsets for name");


/* types */
typedef struct attr_t{
	char const *name;
} attr_t;

typedef struct type_attr_t{
	char const *name;

	expr_type_t type;
	size_t array_limit;

	expr_t *value;
} type_attr_t;

ENSURE_ATTR_TYPE(type_attr_t);

typedef struct node_attr_t{
	char const *name;
	expr_value_t value;
} node_attr_t;

ENSURE_ATTR_TYPE(node_attr_t);


/* prototypes */
type_attr_t *type_attr_init(type_attr_t *attr, char const *name, expr_type_t type, size_t array_limit, expr_t *value);
node_attr_t *node_attr_init(node_attr_t *attr, char const *name, expr_value_t *value);
type_attr_t *type_attr_assign(type_attr_t *attr, expr_t *value);


#endif // DEVTREE_ATTRS_H
