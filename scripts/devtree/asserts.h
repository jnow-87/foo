/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ASSERT_H
#define DEVTREE_ASSERT_H


/* types */
typedef struct assert_t{
	struct assert_t *prev,
					*next;

	char const *expr,
			   *msg;
} assert_t;


/* prototypes */
assert_t *assert_alloc(char const *expr, char const *msg);


#endif // DEVTREE_ASSERT_H
