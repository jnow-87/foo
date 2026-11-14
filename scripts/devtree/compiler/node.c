/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <sys/list.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "expr.h"
#include "node.h"


/* local/static prototypes */
static int index_add(char const *name, node_t *node);
static int index_query(char const *name, size_t *idx);


/* static variables */
static vector_t node_index = VECTOR_INITIALISER(sizeof(node_t*));
static node_t nodes = (node_t){
	.prev = 0x0,
	.next = 0x0,
	.childs = 0x0,
	.name = "root",
	.type = 0x0,
	.attrs = NODE_ATTRVEC_INITIALISER(),
};


/* global functions */
node_t *nodes_root(){
	return &nodes;
}

void nodes_free(void){
	node_t *node;


	list_for_each(nodes.childs, node)
		node_free(node);

	vector_reset(&node_index);
}

node_t *node_alloc(char const *name, type_t *type, node_t *childs){
	node_t *node;


	node = calloc(1, sizeof(node_t));

	if(node == 0x0)
		goto err_0;

	// set a dummy name to avoid segfaults and, in case of an error, avoid
	// node_free() from removing a node with the same name from the index
	node->name = "";
	node->childs = childs;

	if(attrvec_init(&node->attrs.v, sizeof(node_attr_t), type->attrs.v.size) != 0)
		goto err_1;

	if(index_add(name, node) != 0)
		goto err_1;

	node->name = name;
	node->type = type;

	return node;


err_1:
	node_free(node);

err_0:
	devtree_parser_error("%s: node allocation failed", name);

	return 0x0;
}

void node_free(node_t *node){
	size_t idx;
	node_t *child;


	list_for_each(node->childs, child)
		node_free(child);

	if(index_query(node->name, &idx) == 0)
		vector_rm(&node_index, idx);

	node_attrvec_reset(&node->attrs);
	free(node);
}

node_t *node_ref(char const *name){
	size_t idx;


	if(index_query(name, &idx) == -1){
		devtree_parser_error("%s: undefined reference", name);

		return 0x0;
	}

	return *((node_t**)vector_get(&node_index, idx));
}

node_attr_t *node_attr_init(node_attr_t *attr, char const *name, expr_value_t *value){
	if(name == 0x0){
		devtree_parser_error("cannot create unnamed attribute");

		return 0x0;
	}

	attr->name = name;
	attr->value = *value;

	return attr;
}

int node_attr_update(node_t *node, node_attr_t *attr, expr_op_t op, expr_t *arg){
	expr_value_t *v = &attr->value;
	expr_t e = EXPR_CALC(op, arg, &EXPR_LITERAL(*v));
	expr_value_t r;


	if(expr_evaluate(&e, &r, &node->attrs) == 0x0)
		return -1;

	if(expr_type_check(&EXPR_LITERAL(r), v->type, v->is_array, 0) != 0)
		return -1;

	r.type = v->type;
	attr->value = r;

	return asserts_eval(node->type->asserts, &node->attrs, node->name);
}

void node_attrvec_reset(node_attrvec_t *attrs){
	node_attr_t *attr;


	attrvec_for_each(&attrs->v, attr)
		expr_value_reset(&attr->value);

	vector_reset(&attrs->v);
}


/* local functions */
static int index_add(char const *name, node_t *node){
	size_t idx;


	if(index_query(name, &idx) != -1)
		return devtree_parser_error("%s: node already defined", name);

	return vector_add(&node_index, &node);
}

static int index_query(char const *name, size_t *idx){
	node_t **node;


	*idx = 0;

	vector_for_each(&node_index, node){
		if(strcmp((*node)->name, name) == 0)
			return 0;

		(*idx)++;
	}

	return -1;
}
