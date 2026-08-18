/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ASSERT_H
#define DEVTREE_ASSERT_H


#include <stdio.h>
#include "expr.h"


/* macros */
#define ASSERT_TOKEN	256


/* types */
typedef struct assert_t{
	struct assert_t *prev,
					*next;

	expr_t *expr;
	char const *msg;
} assert_t;


/* prototypes */
assert_t *assert_create(expr_t *expr, char const *msg);


#endif // DEVTREE_ASSERT_H
