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
#define ATTRVEC_INITIALISER()	VECTOR_INITIALISER(sizeof(attr_t))
#define attrvec_for_each		vector_for_each


/* incomplete types */
struct attr_t;
enum expr_type_t;


/* types */
typedef vector_t attrvec_t;


/* prototypes */
int attrvec_init(attrvec_t *v, size_t n);
void attrvec_destroy(attrvec_t *v);

int attrvec_add(attrvec_t *v, struct attr_t *attr);

struct attr_t *attrvec_query(attrvec_t *attrs, char const *name, bool maybe_undef);
struct attr_t *attrvec_query_typed(attrvec_t *attrs, char const *name, enum expr_type_t type, bool maybe_undef);



#endif // ATTRVEC_T
