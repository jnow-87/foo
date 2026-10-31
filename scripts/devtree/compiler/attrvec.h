/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ATTRVEC_T
#define ATTRVEC_T


#include <sys/types.h>
#include <sys/vector.h>


/* macros */
#define TYPE_ATTRVEC_INITIALISER()	(type_attrvec_t){ .v = VECTOR_INITIALISER(sizeof(type_attr_t)) }
#define NODE_ATTRVEC_INITIALISER()	(node_attrvec_t){ .v = VECTOR_INITIALISER(sizeof(node_attr_t)) }

#define attrvec_for_each		vector_for_each


/* incomplete types */
struct attr_t;
enum expr_type_t;


/* types */
typedef vector_t attrvec_t;

typedef struct{
	attrvec_t v;
} type_attrvec_t;

typedef struct{
	attrvec_t v;
} node_attrvec_t;


/* prototypes */
int attrvec_alloc(attrvec_t *attrs, size_t type_size, size_t n);
void attrvec_free(attrvec_t *v);

int attrvec_add(attrvec_t *attrs, struct attr_t *attr, size_t attr_size);

struct attr_t *attrvec_query(attrvec_t *attrs, char const *name, bool maybe_undef);
struct type_attr_t *type_attrvec_query(type_attrvec_t *attrs, char const *name, enum expr_type_t type, bool maybe_undef);



#endif // ATTRVEC_T
