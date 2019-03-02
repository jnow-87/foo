/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_NODE_H
#define DEVTREE_NODE_H


#include <sys/vector.h>


/* types */
typedef enum{
	MT_REG_BASE = 1,
	MT_REG_LIST,
	MT_INT_LIST,
} member_type_t;

typedef struct{
	size_t size;
	vector_t *lst;
} member_int_t;

typedef struct{
	member_type_t type;
	void *data;
} member_t;

typedef struct node_t{
	struct node_t *prev,
				  *next,
				  *parent,
				  *childs;

	char const *name,
			   *compatible;

	vector_t data;			/**< vector of member_t elements */
} node_t;


/* prototypes */
char const *node_validate(node_t *n);
void node_export(node_t *n);
void node_print(node_t *n, size_t indent);


#endif // DEVTREE_NODE_H
