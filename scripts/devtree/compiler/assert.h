/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ASSERT_H
#define DEVTREE_ASSERT_H


#include "expr.h"
#include "node.h"


/* types */
typedef struct assert_t{
	struct assert_t *prev,
					*next;

	expr_t *expr;
	char const *msg;
} assert_t;


/* prototypes */
assert_t *assert_alloc(expr_t *expr, char const *msg);
void asserts_free(assert_t *asserts);
int asserts_eval(assert_t *asserts, node_attrvec_t *ctx, char const *obj_name);


#endif // DEVTREE_ASSERT_H
