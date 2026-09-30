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
#include "attrvec.h"
#include "expr.h"
#include "node.h"


/* local/static prototypes */
static int index_add(node_t *node);
static int index_query(char const *name, size_t *idx);


/* static variables */
static vector_t node_index = VECTOR_INITIALISER(sizeof(node_t*));
static node_t nodes = (node_t){
	.prev = 0x0,
	.next = 0x0,
	.childs = 0x0,
	.name = "root",
	.type = 0x0,
	.attrs = ATTRVEC_INITIALISER(),
};


/* global functions */
node_t *nodes_root(){
	return &nodes;
}

void nodes_destroy(void){
	node_t *node;


	list_for_each(nodes.childs, node)
		node_destroy(node);

	vector_destroy(&node_index);
}

node_t *node_create(char const *name, type_t *type, node_t *childs){
	node_t *node,
		   *child;


	node = calloc(1, sizeof(node_t));

	if(node == 0x0)
		goto err_0;

	if(attrvec_init(&node->attrs, type->attrs.size) != 0)
		goto err_1;

	// TODO who should own the name memory, cf. node_destroy(), which free's it
	node->name = name;
	node->type = type;

	list_for_each(childs, child)
		list_add_tail(node->childs, child);

	if(index_add(node) != 0)
		goto err_1;

	return node;


err_1:
	free(node);

err_0:
	devtree_parser_error("%s: node allocation failed", name);

	return 0x0;
}

void node_destroy(node_t *node){
	size_t idx;
	node_t *child;


	list_for_each(node->childs, child)
		node_destroy(child);

	if(index_query(node->name, &idx) == 0)
		vector_rm(&node_index, idx);

	attrvec_destroy(&node->attrs);
	free((char*)node->name);
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

int node_asserts_eval(node_t *node){
	assert_t *assert;
	expr_value_t r;

	// TODO add a test case that checks an assert triggers also after
	// 		an attribute of an existing node is updated
	// TODO consider checking asserts on node creation an each time
	// 		a node's attributes are modified
	list_for_each(node->type->asserts, assert){
		if(expr_evaluate(assert->expr, &r, &node->attrs) == 0x0 || !EXPR_TYPE_IS_INT(r.type) || r.is_array || r.i == 0)
			return devtree_parser_error("assertion failed: %s", assert->msg);
	}

	return 0;
}

int node_attr_update(node_t *node, attr_t *attr, expr_op_t op, expr_t *arg){
	expr_t e = EXPR(op, EXPR_VALUE_EXPR(attr->value), EXPR_VALUE_EXPR(arg));
	expr_value_t r;


	if(expr_evaluate(&e, &r, &node->attrs) == 0x0)
		return -1;

	attr->value->arg0 = r;

	return node_asserts_eval(node);
}


/* local functions */
static int index_add(node_t *node){
	size_t idx;


	if(index_query(node->name, &idx) != -1)
		return devtree_parser_error("%s: node already defined", node->name);

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
