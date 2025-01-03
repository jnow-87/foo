/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/vector.h>
#include <stdlib.h>
#include <string.h>
#include <asserts.h>
#include <attr.h>
#include <types.h>
#include <nodes.h>
#include <parser.tab.h>


/* local/static prototypes */
static int type_validate(type_cat_t category, vector_t *attrs);


/* static variables */
static type_t *type_lst = 0x0;


/* global functions */
type_t *types(void){
	return type_lst;
}

int type_add(char const *name, type_cat_t category, vector_t *attrs, assert_t *asserts){
	type_t *type;


	if(type_validate(category, attrs) != 0)
		return devtree_parser_error("invalid type defintion");

	type = malloc(sizeof(type_t));

	if(type == 0x0)
		return devtree_parser_error("type allocation failed");

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
		devtree_parser_error("unknown type %s", name);

	return type;
}

node_t *type_instantiate(type_t *type, char const *name, vector_t *attrs, node_t *childs){
	attr_t *tattr,
		   *nattr;
	node_t *node;
	attr_value_t v;


	node = node_create(name, type, childs);

	if(node == 0x0)
		goto end;

	vector_for_each(&type->attrs, tattr){
		nattr = attr_get(attrs, tattr->name, true);

		if(nattr == 0x0 && !(tattr->value.flags & AF_HAS_VALUE)){
			devtree_parser_error("%s: missing attribute %s", name, tattr->name);
			goto err;
		}

		if(nattr != 0x0 && (attr_type_check(&nattr->value, tattr->value.type) != 0 || attr_range_check(tattr, &nattr->value) != 0))
			goto err;

		v = nattr ? nattr->value : tattr->value;
		v.type = tattr->value.type;
		v.flags = tattr->value.flags;

		if(attr_assign(&node->attrs, tattr->name, &v) != 0){
			devtree_parser_error("%s: adding attribute failed", name);
			goto err;
		}
	}

	// TODO explicitly print the invalid attributes
	if(type->attrs.size == node->attrs.size)
		goto end;

	devtree_parser_error("invalid attributes");


err:
	node_destroy(node);
	node = 0x0;

end:
	vector_destroy(attrs);

	return node;
}

char const *type_strcat(type_cat_t category){
	switch(category){
	case TC_MEMORY:	return "memory";
	case TC_DEVICE:	return "device";
	default:		return "invalid";
	}
}


/* local functions */
static int type_validate(type_cat_t category, vector_t *attrs){
	if(category == TC_DEVICE){
		if(attr_get_typed(attrs, "compatible", MT_STRING, false) == 0x0)
			return -1;
	}

	return 0;
}
