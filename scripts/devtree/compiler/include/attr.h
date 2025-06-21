/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ATTR_H
#define DEVTREE_ATTR_H


#include <stdbool.h>
#include <sys/types.h>
#include <sys/vector.h>


/* macros */
#define ATTR_ARRAY_UNLIMITED	((size_t)-1)
#define ATTR_INT_TYPE			unsigned long int

#define ATTR_VALUE(member, value)			(attr_value_t){ .member = value }
#define ATTR_UNNAMED(type, member, value)	attr_init(&(attr_t){}, 0x0, type, 0, &ATTR_VALUE(member, value))
#define ATTR_UNNAMED_INT(value)				ATTR_UNNAMED(AT_INT64, i, value)
#define ATTR_UNNAMED_STRING(value)			ATTR_UNNAMED(AT_STRING, p, value)


/* types */
typedef enum{
	AT_UNDEF = 0,
	AT_INT8,
	AT_INT16,
	AT_INT32,
	AT_INT64,
	AT_ADDR,
	AT_STRING,
} attr_type_t;

typedef enum{
	AF_NONE = 0x0,
	AF_HAS_VALUE = 0x1,
	AF_ARRAY = 0x2,
} attr_flags_t;

typedef enum{
	OP_ADD = 0,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_LSHIFT,
	OP_RSHIFT,
	OP_MOD,
} attr_op_t;

typedef struct{
	vector_t items;
	size_t limit;
} attr_array_t;

typedef union{
	void *p;
	ATTR_INT_TYPE i;
	attr_array_t arr;
} attr_value_t;

typedef struct{
	char const *name;

	attr_type_t type;
	attr_flags_t flags;
	attr_value_t value;
} attr_t;


/* prototypes */
attr_t *attr_init(attr_t *attr, char const *name, attr_type_t type, size_t array_limit, attr_value_t *value);

int attr_enlist(vector_t *attrs, attr_t *attr);
attr_t *attr_assign(attr_t *attr, attr_t *value);
attr_t *attr_math(attr_t *a0, attr_t *a1, attr_op_t op, bool resolve);
int attr_copy(attr_t *dest, attr_t *src);

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef);
attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef);

attr_t *attr_convert_to_list(attr_t *attr);
char const *attr_type_name(attr_type_t type);
size_t attr_type_size(attr_type_t type);


#endif // DEVTREE_ATTR_H
