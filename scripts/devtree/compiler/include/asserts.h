/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ASSERT_H
#define DEVTREE_ASSERT_H


#include <stdio.h>
#include <sys/vector.h>


/* macros */
#define ASSERT_TOKEN	256


/* types */
typedef struct assert_t{
	struct assert_t *prev,
					*next;

	char const *expr,
			   *msg;
} assert_t;


/* prototypes */
assert_t *assert_create(char const *expr, char const *msg);
int assert_check(assert_t *assert, vector_t *attrs);
int assert_export(FILE *fp, assert_t *assert, vector_t *attrs);


#endif // DEVTREE_ASSERT_H
