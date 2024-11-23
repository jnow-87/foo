/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ATTR_H
#define DEVTREE_ATTR_H


#include <sys/types.h>
#include <stdbool.h>


/* macros */
#define ATTR_TYPE_STR_BIT	31
#define ATTR_TYPE_INT_BIT	30

#define ATTR_TYPE_ISINT(type)	(((type) & MT_INT) && (((type) & ~MT_INT) < MT_INT_MAX))
#define ATTR_TYPE_ISSTR(type)	(((type) & MT_STRING) && (((type) & ~MT_STRING) == 0))

#define ATTR_VALUE(type, value) \
	(attr_value_t){ .type = value }


/* types */
typedef enum{
	// base type
	MT_UNDEF = 0,
	MT_INT = 0x1 << ATTR_TYPE_INT_BIT,
	MT_STRING = 0x1 << ATTR_TYPE_STR_BIT,

	// specific types
	MT_ADDR = MT_INT + 1,
	MT_INT8,
	MT_INT16,
	MT_INT32,
	MT_INT64,

	MT_INT_MAX,
} attr_type_t;

typedef union{
	void *p;
	unsigned long int i;
} attr_value_t;

typedef struct{
	char const *name;

	attr_type_t type;
	attr_value_t value;
	bool value_set;
} attr_t;


/* prototypes */
int attr_add(vector_t *attrs, char const *name, attr_type_t type, attr_value_t *value);
attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef);
attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef);

int attr_type_check(attr_t *attr, attr_type_t type);
char const *attr_strtype(attr_type_t type);


#endif // DEVTREE_ATTR_H
