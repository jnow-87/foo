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

#define ATTR_VALUE(member, _value, _type, _size) \
	(attr_value_t){ \
		.type = _type, \
		.size = _size, \
		.member = _value, \
		.flags = AF_HAS_VALUE, \
	}

#define ATTR_NOVALUE(_type, _size) \
	(attr_value_t){ \
		.type = _type, \
		.size = _size, \
		.flags = AF_NONE, \
	}

#define ATTR_VALUE_ADDR(value)			ATTR_VALUE(p, value, MT_ADDR, ATTR_INT_SIZE)
#define ATTR_VALUE_INT(value, size)		ATTR_VALUE(i, value, MT_INT, size)
#define ATTR_VALUE_ILIST(value, size)	ATTR_VALUE(v, value, MT_ILIST, size)
#define ATTR_VALUE_STRING(value)		ATTR_VALUE(p, value, MT_STRING, 1)
#define ATTR_ADDR()						ATTR_NOVALUE(MT_ADDR, ATTR_INT_SIZE)
#define ATTR_INT(size)					ATTR_NOVALUE(MT_INT, size)
#define ATTR_ILIST(size)				ATTR_NOVALUE(MT_ILIST, size)
#define ATTR_STRING()					ATTR_NOVALUE(MT_STRING, 1)


/* types */
typedef enum{
	MT_UNDEF = 0,
	MT_ADDR,
	MT_INT,
	MT_ILIST,
	MT_STRING,
} attr_type_t;

typedef enum{
	AF_NONE = 0x0,
	AF_HAS_VALUE = 0x1,
} attr_flags_t;

typedef struct{
	attr_type_t type;
	size_t size;
	attr_flags_t flags;

	union{
		void *p;
		ATTR_INT_TYPE i;
		vector_t v;
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

int attr_ilist_add(vector_t *lst, ATTR_INT_TYPE value);


#endif // DEVTREE_ATTR_H
