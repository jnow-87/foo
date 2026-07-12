/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ATTRS_H
#define DEVTREE_ATTRS_H


#include <stdbool.h>
#include <sys/types.h>
#include <sys/vector.h>
#include "expr.h"



/**
 * TODO
 *   should there be two types of attributes, one for nodes one for types
 *   type attributes allow expressions
 *   node attributes allow only literal expression, i.e. expr_arg_t
 *
 *   type, array_limit and has_value should then only be needed for type attributes
 *   has_value should be removable, since value is now a pointer
 */



/* macros */
typedef struct{
	char const *name;

	expr_type_t type;
	size_t array_limit;
	bool has_value;

	expr_t *value;
} attr_t;

typedef vector_t attr_vec_t;


/* prototypes */
attr_t *attr_init(attr_t *attr, char const *name, expr_type_t type, size_t array_limit, expr_t *value);

int attr_enlist(attr_vec_t *attrs, attr_t *attr);
attr_t *attr_assign(attr_t *attr, expr_t *value);

attr_t *attr_query(attr_vec_t *attrs, char const *name, bool maybe_undef);
attr_t *attr_query_typed(attr_vec_t *attrs, char const *name, expr_type_t type, bool maybe_undef);

expr_arg_t *attr_value(attr_t *attr);


#endif // DEVTREE_ATTRS_H
