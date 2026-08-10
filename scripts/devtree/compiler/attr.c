/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/compiler.h>
#include <sys/math.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"


/* global functions */
attr_t *attr_init(attr_t *attr, char const *name, expr_type_t type, size_t array_limit, expr_t *value){
	if(name == 0x0){
		devtree_parser_error("cannot create unnamed attribute");

		return 0x0;
	}

	attr->name = name;
	attr->type = type;
	attr->array_limit = array_limit;
	attr->has_value = false;

	return attr_assign(attr, value);
}

int attr_enlist(attr_vec_t *attrs, attr_t *attr){
	if(attr_query(attrs, attr->name, true) != 0x0)
		return devtree_parser_error("%s: attribute already defined", attr->name);

	if(vector_add(attrs, attr) != 0)
		return devtree_parser_error("%s: adding attribute failed", attr->name);

	return 0;
}

attr_t *attr_assign(attr_t *attr, expr_t *value){
	if(value == 0x0)
		return attr;

	if(expr_type_check(value, attr->type) != 0)
		return 0x0;

/* TODO is it needed to check or update the array limit here?
	if((attr->flags & AF_ARRAY) && attr->value.arr.limit != ATTR_ARRAY_UNLIMITED)
		v.arr.limit = attr->value.arr.limit;
*/
	attr->value = value;

	return attr;
}

attr_t *attr_query(attr_vec_t *attrs, char const *name, bool maybe_undef){
	return attr_query_typed(attrs, name, ET_UNDEF, maybe_undef);
}

attr_t *attr_query_typed(attr_vec_t *attrs, char const *name, expr_type_t type, bool maybe_undef){
	attr_t *attr;


	vector_for_each(attrs, attr){
		if(strcmp(attr->name, name) == 0){
			if(type == ET_UNDEF || attr->type == type)
				return attr;

			devtree_parser_error("%s: invalid type %s, expecting %s", name, expr_type_name(attr->type), expr_type_name(type));

			return 0x0;
		}
	}

	if(!maybe_undef)
		devtree_parser_error("%s: undefined attribute", name);

	return 0x0;
}

expr_value_t *attr_value(attr_t *attr){
	// TODO should the function instead call expr_evaluate() to avoid the check for expr_literal
	// 		and be callable for all attribute, either ones with literal and non-literal values
	if(attr->value->op != expr_literal){
		devtree_parser_error("%s: attribute value is not a literal", attr->name);

		return 0x0;
	}

	return attr->value->arg0;
}
