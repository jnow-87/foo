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


/* macros */
#define ENSURE_ATTR_TYPE(type) \
	_Static_assert(offsetof(attr_t, name) == offsetofvar(type, name), \
		#type " is not compatible with attr_t, due to mismatching offsets for name");

#define attrvec_for_each vector_for_each


/* types */
typedef struct{
	char const *name;
} attr_t;

typedef vector_t attrvec_t;


/* prototypes */
int attrvec_init(attrvec_t *attrs, size_t type_size, size_t n);

int attrvec_add(attrvec_t *attrs, attr_t *attr, size_t attr_size);
attr_t *attrvec_query(attrvec_t *attrs, char const *name, bool maybe_undef);


#endif // DEVTREE_ATTRS_H
