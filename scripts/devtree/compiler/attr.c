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
int attr_add(vector_t *attrs, char const *name, attr_type_t type, attr_flags_t flags, attr_value_t *value){
	attr_t attr;


	attr.name = name;
	attr.type = type;
	attr.flags = flags;
	attr.flags |= ((type == MT_STRING) ? AF_STRING : AF_INT);

	if(value != 0x0){
		attr.flags |= AF_HAS_VALUE;
		attr.value = *value;

		if(attr_range_check(&attr, *value) != 0)
			return -1;
	}

	if(attr_type_check(&attr, attr.type, attr.flags) != 0)
		return -1;

	if(vector_add(attrs, &attr) != 0)
		return devtree_parser_error("%s adding attribute failed", name);

	return 0;
}

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef){
	return attr_get_typed(attrs, name, MT_UNDEF, maybe_undef);
}

attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef){
	attr_t *attr;


	vector_for_each(attrs, attr){
		if(strcmp(attr->name, name) == 0){
			if(type == MT_UNDEF || attr_type_check(attr, type, attr->flags) == 0)
				return attr;

			devtree_parser_error("%s: invalid type %s, expecting %s", name, attr_strtype(attr->type), attr_strtype(type));

			return 0x0;
		}
	}

	if(!maybe_undef)
		devtree_parser_error("undefined attribute \"%s\"", name);

	return 0x0;
}

int attr_type_check(attr_t *attr, attr_type_t type, attr_flags_t flags){
	if(type == MT_UNDEF || (((flags & AF_INT) && (flags & AF_STRING)) || !(flags & (AF_INT | AF_STRING))))
		return devtree_parser_error("%s attribute has to be either string or integer", attr->name);

	if(ATTR_FLAGS_TYPE_MASK(attr->flags) != ATTR_FLAGS_TYPE_MASK(flags) || (attr->type != type && !(flags & AF_INT)))
		return devtree_parser_error("type mismatch have %s expected %s", attr_strtype(type), attr_strtype(attr->type));

	return 0;
}

int attr_range_check(attr_t *attr, attr_value_t value){
	unsigned long int lim;
	unsigned long int *v;


	if(attr->type < MT_INT8 || attr->type > MT_INT64)
		return 0;

	lim = (((unsigned long int)1 << (attr_int_size(attr->type) - 1)) << 1) - 1;

	if(attr->flags & AF_LIST){
		vector_for_each(&value.v, v){
			if(*v > lim)
				return devtree_parser_error("%s out of range %lu > %lu", attr->name, *v, lim);
		}
	}
	else if(value.i > lim)
		return devtree_parser_error("%s out of range %lu > %lu", attr->name, value.i, lim);

	return 0;
}

char const *attr_strtype(attr_type_t type){
	static char const *names[] = {
		"unknown",
		"string",
		"addr",
		"int8",
		"int16",
		"int32",
		"int64",
	};

	if(type < 0 || type > MT_INT64)
		type = MT_UNDEF;

	return names[type];
}

size_t attr_int_size(attr_type_t type){
	switch(type){
	case MT_INT8:	return 8;
	case MT_INT16:	return 16;
	case MT_INT32:	return 32;
	case MT_INT64:	return 64;
	default:		return 0;
	}
}

int ilist_add(vector_t *lst, unsigned long int v){
	if(vector_add(lst, &v) != 0)
		return devtree_parser_error("intlist extension failed");

	return 0;
}
