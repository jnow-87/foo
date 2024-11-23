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
#define ATTR_FLAGS_TYPE_MASK(flags)	((flags) & ~(AF_HAS_VALUE))
#define ATTR_VALUE(type, value) \
	(attr_value_t){ .type = value }


/* types */
typedef enum{
	AF_NONE = 0x0,
	AF_INT = 0x1,
	AF_STRING = 0x2,
	AF_LIST = 0x4,
	AF_HAS_VALUE = 0x8,
} attr_flags_t;

typedef enum{
	MT_UNDEF = 0,
	MT_STRING,
	MT_ADDR,
	MT_INT8,
	MT_INT16,
	MT_INT32,
	MT_INT64,
} attr_type_t;

typedef union{
	void *p;
	unsigned long int i;
	vector_t v;
} attr_value_t;

typedef struct{
	char const *name;

	attr_type_t type;
	attr_value_t value;
	attr_flags_t flags;
} attr_t;


/* prototypes */
int attr_add(vector_t *attrs, char const *name, attr_type_t type, attr_flags_t flags, attr_value_t *value);
attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef);
attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef);

int attr_type_check(attr_t *attr, attr_type_t type, attr_flags_t flags);
char const *attr_strtype(attr_type_t type);

size_t attr_int_size(attr_type_t type);

int ilist_add(vector_t *lst, unsigned long int v);


#endif // DEVTREE_ATTR_H
