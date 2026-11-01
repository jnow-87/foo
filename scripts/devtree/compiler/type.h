/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_TYPES_H
#define DEVTREE_TYPES_H


#include <sys/types.h>
#include <sys/vector.h>
#include "assert.h"
#include "attr.h"
#include "expr.h"
#include "node.h"


/* macros */
#define TYPE_ATTRVEC_INITIALISER()	(type_attrvec_t){ .v = VECTOR_INITIALISER(sizeof(type_attr_t)) }


/* types */
typedef struct type_attr_t{
	char const *name;

	expr_type_t type;
	size_t array_limit;

	expr_t *value;
} type_attr_t;

ENSURE_ATTR_TYPE(type_attr_t);

typedef struct{
	attrvec_t v;
} type_attrvec_t;

typedef struct type_t{
	struct type_t *prev,
				  *next;

	char const *name;
	assert_t *asserts;
	type_attrvec_t attrs;
} type_t;


/* prototypes */
int type_alloc(char const *name, type_attrvec_t *attrs, assert_t *asserts);
void types_free(void);

type_t *type_lookup(char const *name);
node_t *type_instantiate(type_t *type, char const *name, node_attrvec_t *attrs, node_t *childs);

type_attr_t *type_attr_init(type_attr_t *attr, char const *name, expr_type_t type, size_t array_limit, expr_t *value);
type_attr_t *type_attr_assign(type_attr_t *attr, expr_t *value);

struct type_attr_t *type_attrvec_query(type_attrvec_t *attrs, char const *name, enum expr_type_t type, bool maybe_undef);


#endif // DEVTREE_TYPES_H
