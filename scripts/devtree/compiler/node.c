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
#include "node.h"


/* local/static prototypes */
static int eval_asserts(node_t *node);

static int index_add(node_t *node);
static node_t *index_query(char const *name);


/* static variables */
static vector_t node_index = VECTOR_INITIALISER(sizeof(node_t*));
static node_t *nodes;


/* global functions */
int nodes_init(void){
	attrvec_t attrs = ATTRVEC_INITIALISER();


	if(attrvec_add(&attrs, attr_init(&(attr_t){}, "compatible", ET_STRING, 0, &EXPR_STR(""))) != 0)
		return -1;

	if(type_create("root", &attrs, 0x0) != 0)
		return -1;

	nodes = type_instantiate(type_lookup("root"), "root", &attrs, 0x0);

	if(nodes == 0x0)
		return -1;

	return 0;
}

node_t *nodes_root(){
	return nodes;
}

int nodes_assert(void){
	node_t *node;


	list_for_each(nodes, node){
		if(eval_asserts(node) != 0)
			return -1;
	}

	return 0;
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
		node_child_add(node, child);

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
	attrvec_destroy(&node->attrs);
	free((char*)node->name);
	free(node);
}

void node_child_add(node_t *parent, node_t *child){
	child->parent = parent;
	list_add_tail(parent->childs, child);
}

node_t *node_ref(char const *name){
	node_t *node;


	node = index_query(name);

	if(node == 0x0)
		devtree_parser_error("%s: undefined reference", name);

	return node;
}


/* local functions */
static int index_add(node_t *node){
	if(index_query(node->name) != 0x0)
		return devtree_parser_error("%s: node already defined", node->name);

	return vector_add(&node_index, &node);
}

static node_t *index_query(char const *name){
	node_t **node;


	vector_for_each(&node_index, node){
		if(strcmp((*node)->name, name) == 0)
			return *node;
	}

	return 0x0;
}

static int eval_asserts(node_t *node){
	assert_t *assert;
	expr_value_t r;

	// TODO add a test case that checks an assert triggers also after
	// 		an attribute of an existing node is updated
	// TODO consider checking asserts on node creation an each time
	// 		a node's attributes are modified
	list_for_each(node->type->asserts, assert){
		if(expr_evaluate(assert->expr, &r, &node->attrs) == 0x0 || r.is_array || r.i != 0)
			return -1;
	}

	return 0;
}
