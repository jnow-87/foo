/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <sys/list.h>
#include <sys/vector.h>
#include <asserts.h>
#include <attr.h>
#include <nodes.h>
#include <parser.tab.h>
#include <types.h>


/* local/static prototypes */
static int type_validate(type_cat_t category, vector_t *attrs, assert_t *asserts);


/* static variables */
static type_t *type_lst = 0x0;


/* global functions */
type_t *types(void){
	return type_lst;
}

int type_add(char const *name, type_cat_t category, vector_t *attrs, assert_t *asserts){
	type_t *type;


	if(type_validate(category, attrs, asserts) != 0)
		return devtree_parser_error("%s: invalid type definition", name);

	type = malloc(sizeof(type_t));

	if(type == 0x0)
		return devtree_parser_error("%s: type allocation failed", name);

	type->name = name;
	type->category = category;
	type->attrs = *attrs;
	type->asserts = asserts;

	list_add_tail(type_lst, type);

	return 0;
}

type_t *type_lookup(char const *name){
	type_t *type;


	type = list_find_str(type_lst, name, name);

	if(type == 0x0)
		devtree_parser_error("%s: unknown type", name);

	return type;
}

node_t *type_instantiate(type_t *type, char const *name, vector_t *attrs, node_t *childs){
	attr_t *tattr,
		   *nattr;
	attr_t attr;
	node_t *node;


	node = node_create(name, type, childs);

	if(node == 0x0)
		goto err;

	/* check for invalid attributes */
	vector_for_each(attrs, nattr){
		if(attr_get(&type->attrs, nattr->name, true) != 0x0)
			continue;

		devtree_parser_error("%s: undefined attribute %s for type %s", name, nattr->name, type->name);
		goto err;
	}

	/* create node attribute list */
	vector_for_each(&type->attrs, tattr){
		nattr = attr_get(attrs, tattr->name, true);

		if(nattr == 0x0 && !(tattr->flags & AF_HAS_VALUE)){
			devtree_parser_error("%s: missing attribute %s", name, tattr->name);
			goto err;
		}

//		if(nattr != 0x0 && (!attr_type_compatible(tattr, nattr) || attr_range_check(tattr, &nattr->value) != 0))
//			goto err;

		attr = *tattr;

		if(nattr != 0x0 && attr_assign(&attr, nattr) == 0x0)
			goto err;

		if(attr_enlist(&node->attrs, &attr) != 0){
			devtree_parser_error("%s: adding attribute failed", name);
			goto err;
		}
	}

	vector_destroy(attrs);

	return node;


err:
	if(node != 0x0)
		node_destroy(node);

	vector_destroy(attrs);

	return 0x0;
}

char const *type_strcat(type_cat_t category){
	switch(category){
	case TC_MEMORY:	return "memory";
	case TC_DEVICE:	return "device";
	default:		return "invalid";
	}
}


/* local functions */
static int type_validate(type_cat_t category, vector_t *attrs, assert_t *asserts){
	assert_t *assert;


	if(category == TC_DEVICE){
		if(attr_get_typed(attrs, "compatible", AT_STRING, false) == 0x0)
			return -1;
	}

	list_for_each(asserts, assert){
		if(assert_check(assert, attrs) != 0)
			return -1;
	}

	return 0;
}
