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

attr_t *attr_assign(attr_t *attr, expr_t *value){
	if(value == 0x0)
		return attr;

	if(expr_type_check(value, attr->type) != 0)
		return 0x0;

/* TODO is it needed to
 * 	- check the array limit: yes, only if it matches the assignment is allowed
 * 	- update the array limit: unclear
 *
	if((attr->flags & AF_ARRAY) && attr->value.arr.limit != ATTR_ARRAY_UNLIMITED)
		v.arr.limit = attr->value.arr.limit;
*/
	attr->value = value;

	return attr;
}

expr_value_t *attr_value(attr_t *attr){
	// TODO should the function instead call expr_evaluate() to avoid the check for EOP_LITERAL
	// 		and be callable for all attribute, either ones with literal and non-literal values
	if(attr->value->op != EOP_LITERAL){
		devtree_parser_error("%s: attribute value is not a literal", attr->name);

		return 0x0;
	}

	return &attr->value->arg0;
}
