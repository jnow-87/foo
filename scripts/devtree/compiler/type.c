/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <sys/list.h>
#include <parser.tab.h>
#include "assert.h"
#include "attr.h"
#include "expr.h"
#include "node.h"
#include "type.h"


/* local/static prototypes */
static int lint(type_attrvec_t *attrs);


/* static variables */
static type_t *types = 0x0;


/* global functions */
int type_alloc(char const *name, type_attrvec_t *attrs, assert_t *asserts){
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

void types_free(void){
	type_t *type;


	list_for_each(types, type){
		asserts_free(type->asserts);
		attrvec_free(&type->attrs.v);
		free(type);
	}
}

type_t *type_lookup(char const *name){
	type_t *type;


	type = list_find_str(types, name, name);

	if(type == 0x0)
		devtree_parser_error("%s: unknown type", name);

	return type;
}

node_t *type_instantiate(type_t *type, char const *name, node_attrvec_t *attrs, node_t *childs){
	type_attr_t *tattr;
	node_attr_t *nattr;
	node_attr_t attr;
	node_t *node;


	/* check attributes */
	// ensure no additonal attributes are defined
	attrvec_for_each(&attrs->v, nattr){
		if(attrvec_query(&type->attrs.v, nattr->name, true) != 0x0)
			continue;

		devtree_parser_error("%s: undefined attribute %s for type %s", name, nattr->name, type->name);
		goto err_0;
	}

	// ensure all required attributes are defined
	attrvec_for_each(&type->attrs.v, tattr){
		nattr = (node_attr_t*)attrvec_query(&attrs->v, tattr->name, true);

		if(nattr == 0x0 && tattr->value == 0x0){
			devtree_parser_error("%s: missing attribute %s", name, tattr->name);
			goto err_0;
		}

		// TODO update once type checking is final
//		if(nattr != 0x0 && (!attr_type_compatible(tattr, nattr) || attr_range_check(tattr, &nattr->value) != 0))
//			goto err;
	}

	/* create node */
	node = node_alloc(name, type, childs);

	if(node == 0x0)
		goto err_0;

	// create node attribute list
	attrvec_for_each(&type->attrs.v, tattr){
		nattr = (node_attr_t*)attrvec_query(&attrs->v, tattr->name, true);

		if(nattr == 0x0){
			attr.name = tattr->name;

			if(expr_evaluate(tattr->value, &attr.value, &node->attrs) == 0x0){
				devtree_parser_error("%s: cannot evaluate expression", nattr->name);
				goto err_1;
			}
		}
		else{
			attr = *nattr;
			nattr->value.type = ET_UNDEF;
		}

		if(attrvec_add(&node->attrs.v, (attr_t*)&attr, sizeof(node_attr_t)) != 0)
			goto err_2;
	}

	if(node_asserts_eval(node) != 0)
		goto err_1;

	return node;


err_2:
	expr_value_reset(&attr.value);

err_1:
	// TODO check if a separate label is needed to jump to from within the loop
	// 		to only free expr and attr.name
	node_free(node);

err_0:
	return 0x0;
}


/* local functions */
static int lint(type_attrvec_t *attrs){
	int missing = 0;
	type_attr_t *comp;


	comp = type_attrvec_query(attrs, "compatible", ET_STRING, false);

	if(comp == 0x0)
		return -1;

	if(strcmp(comp->name, "memory") == 0){
		missing |= (type_attrvec_query(attrs, "base", ET_ADDR, false) == 0x0);
		missing |= (type_attrvec_query(attrs, "size", ET_INT32, false) == 0x0);
	}

	return -missing;
}
