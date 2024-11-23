/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <stdlib.h>
#include <string.h>
#include <attr.h>
#include <parser.tab.h>


/* global functions */
int attr_add(vector_t *attrs, char const *name, attr_type_t type, attr_value_t *value){
	attr_t attr;


	attr.name = name;
	attr.type = type;
	attr.value_set = (value != 0x0);

	if(attr.value_set)
		attr.value = *value;

	if(vector_add(attrs, &attr) != 0)
		return devtree_parser_error("adding attribute %s failed", name);

	return 0;
}

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef){
	return attr_get_typed(attrs, name, MT_UNDEF, maybe_undef);
}

attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef){
	attr_t *attr;


	vector_for_each(attrs, attr){
		if(strcmp(attr->name, name) == 0){
			if(type == MT_UNDEF || attr->type == type)
				return attr;

			devtree_parser_error("%s: invalid type %s, expecting %s", name, attr_strtype(attr->type), attr_strtype(type));

			return 0x0;
		}
	}

	if(!maybe_undef)
		devtree_parser_error("undefined attribute \"%s\"", name);

	return 0x0;
}

int attr_type_check(attr_t *attr, attr_type_t type){
	if(attr->type == type || (ATTR_TYPE_ISINT(attr->type) && ATTR_TYPE_ISINT(type)))
		return 0;

	return devtree_parser_error("type mismatch have %s expected %s", attr_strtype(type), attr_strtype(attr->type));
}

char const *attr_strtype(attr_type_t type){
	static char const *names[] = {
		"unknown",
		"addr",
		"int8",
		"int16",
		"int32",
		"int64",
		"size",
		"addr-width",
		"reg-width",
		"ncores",
		"num-ints",
		"timer-int",
		"syscall-int",
		"ipi-int",
		"timer-cycle-time-us",
		"core-mask",
	};


	if(!ATTR_TYPE_ISINT(type) && !ATTR_TYPE_ISSTR(type))
		return names[0];

	return (type & MT_STRING) ? "string" : names[type & ~MT_INT];
}
