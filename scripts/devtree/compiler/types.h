/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_TYPES_H
#define DEVTREE_TYPES_H


#include <sys/vector.h>
#include <stdbool.h>
#include <asserts.h>
#include <attr.h>


/* incomplete types */
struct node_t;


/* types */
typedef enum{
	TC_MEMORY = 0x1,
	TC_DEVICE = 0x2,
} type_cat_t;

typedef struct type_t{
	struct type_t *prev,
				  *next;

	char const *name;
	type_cat_t category;

	assert_t *asserts;
	vector_t attrs;
} type_t;


/* prototypes */
type_t *types(void);

int type_add(char const *name, type_cat_t category, vector_t *attrs, assert_t *asserts);
type_t *type_lookup(char const *name);
struct node_t *type_instantiate(type_t *type, char const *name, vector_t *attrs, struct node_t *childs);

char const *type_strcat(type_cat_t category);


#endif // DEVTREE_TYPES_H
