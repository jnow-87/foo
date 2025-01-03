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
#define ATTR_INT_TYPE	unsigned long int
#define ATTR_INT_SIZE	sizeof(ATTR_INT_TYPE)

#define ATTR_VALUE(member, _value, _type) \
	(attr_value_t){ \
		.type = _type, \
		.member = _value, \
		.flags = AF_HAS_VALUE, \
	}

#define ATTR_NOVALUE(_type) \
	(attr_value_t){ \
		.type = _type, \
		.flags = AF_NONE, \
	}

#define ATTR_ILIST(_limit) \
	(attr_ilist_t){ \
		.items = VECTOR_INITIALISER(ATTR_INT_SIZE), \
		.limit = _limit, \
	}

#define ATTR_VALUE_ADDR(value)			ATTR_VALUE(p, value, MT_ADDR)
#define ATTR_VALUE_INT(value, type)		ATTR_VALUE(i, value, type)
#define ATTR_VALUE_ILIST(value)			ATTR_VALUE(ilist, value, MT_ILIST)
#define ATTR_VALUE_STRING(value)		ATTR_VALUE(p, value, MT_STRING)


/* types */
typedef enum{
	MT_UNDEF = 0,
	MT_INT8,
	MT_INT16,
	MT_INT32,
	MT_INT64,
	MT_ADDR,
	MT_ILIST,
	MT_STRING,
} attr_type_t;

typedef enum{
	AF_NONE = 0x0,
	AF_HAS_VALUE = 0x1,
} attr_flags_t;

typedef struct{
	vector_t items;
	size_t limit;
} attr_ilist_t;

typedef struct{
	attr_type_t type;
	attr_flags_t flags;

	union{
		void *p;
		ATTR_INT_TYPE i;
		attr_ilist_t ilist;
	};
} attr_value_t;

typedef struct{
	char const *name;

	attr_value_t value;
} attr_t;


/* prototypes */
int attr_init(attr_t *attr, char const *name, attr_value_t *value);
int attr_assign(vector_t *attrs, char const *name, attr_value_t *value);
int attr_add(attr_value_t *value, attr_value_t *op);
int attr_copy(attr_value_t *dest, attr_value_t *src);

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef);
attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef);

int attr_type_check(attr_value_t *valie, attr_type_t type);
int attr_range_check(attr_t *attr, attr_value_t *value);
char const *attr_strtype(attr_type_t type);

int attr_ilist_add(attr_ilist_t *lst, ATTR_INT_TYPE value);

bool attr_is_int(attr_type_t type);
size_t attr_type_size(attr_type_t type);


#endif // DEVTREE_ATTR_H
