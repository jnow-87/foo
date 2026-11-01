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
type_attr_t *type_attr_init(type_attr_t *attr, char const *name, expr_type_t type, size_t array_limit, expr_t *value){
	if(name == 0x0){
		devtree_parser_error("cannot create unnamed attribute");

		return 0x0;
	}

	attr->name = name;
	attr->type = type;
	attr->array_limit = array_limit;

	return type_attr_assign(attr, value);
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

type_attr_t *type_attr_assign(type_attr_t *attr, expr_t *value){
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
