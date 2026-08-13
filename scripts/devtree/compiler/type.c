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
#include <parser.tab.h>
#include "assert.h"
#include "attr.h"
#include "node.h"
#include "type.h"


/* local/static prototypes */
static int lint(vector_t *attrs);


/* static variables */
static type_t *types = 0x0;


/* global functions */
int type_create(char const *name, vector_t *attrs, assert_t *asserts){
	type_t *type;


	if(lint(attrs) != 0)
		return devtree_parser_error("%s: invalid type definition", name);

	type = malloc(sizeof(type_t));

	if(type == 0x0)
		return devtree_parser_error("%s: type allocation failed", name);

	type->name = name;
	type->attrs = *attrs;
	type->asserts = asserts;

	list_add_tail(types, type);

	return 0;
}

type_t *type_lookup(char const *name){
	type_t *type;


	type = list_find_str(types, name, name);

	if(type == 0x0)
		devtree_parser_error("%s: unknown type", name);

	return type;
}

node_t *type_instantiate(type_t *type, char const *name, vector_t *attrs, node_t *childs){
	attr_t *tattr,
		   *nattr;
	attr_t attr;
	node_t *node;


	/* check attributes */
	// ensure no additonal attributes are defined
	vector_for_each(attrs, nattr){
		if(attr_query(&type->attrs, nattr->name, true) != 0x0)
			continue;

		devtree_parser_error("%s: undefined attribute %s for type %s", name, nattr->name, type->name);
		goto err_0;
	}

	// ensure all required attributes are defined
	vector_for_each(&type->attrs, tattr){
		nattr = attr_query(attrs, tattr->name, true);

		if(nattr == 0x0 && !tattr->has_value){
			devtree_parser_error("%s: missing attribute %s", name, tattr->name);
			goto err_0;
		}

		// TODO update once type checking is final
//		if(nattr != 0x0 && (!attr_type_compatible(tattr, nattr) || attr_range_check(tattr, &nattr->value) != 0))
//			goto err;
	}

	/* create node */
	node = node_create(name, type, childs);

	if(node == 0x0)
		goto err_0;

	// create node attribute list
	vector_for_each(&type->attrs, tattr){
		nattr = attr_query(attrs, tattr->name, true);
		attr = *tattr;

		// TODO check if this check should be here or higher up in the function
		// 		EOP_* should not be refernced outside of expr.c and the parser
		if(nattr->value->op != EOP_LITERAL){
			devtree_parser_error("%s: cannot assign non-literal expression", nattr->name);
			goto err_1;
		}

		if(nattr != 0x0 && attr_assign(&attr, nattr->value) == 0x0)
			goto err_1;

		if(attr_enlist(&node->attrs, &attr) != 0)
			goto err_1;
	}

	vector_destroy(attrs);

	return node;


err_1:
	node_destroy(node);

err_0:
	return 0x0;
}


/* local functions */
static int lint(vector_t *attrs){
	int missing = 0;
	attr_t *comp;


	comp = attr_query_typed(attrs, "compatible", ET_STRING, false);

	if(comp == 0x0)
		return -1;

	if(strcmp(comp->name, "memory") == 0){
		missing |= (attr_query_typed(attrs, "base", ET_ADDR, false) == 0x0);
		missing |= (attr_query_typed(attrs, "size", ET_INT32, false) == 0x0);
	}

	return -missing;
}
