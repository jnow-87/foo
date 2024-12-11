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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <attr.h>
#include <parser.tab.h>


/* global functions */
int attr_init(attr_t *attr, char const *name, attr_value_t *value){
	attr->name = name;

	if(value == 0x0)
		return 0;

	if(attr_range_check(attr, value) != 0)
		return -1;

	attr->value = *value;

	return 0;
}

int attr_assign(vector_t *attrs, char const *name, attr_value_t *value){
	attr_t attr;


	if(attr_get(attrs, name, true) != 0x0)
		return devtree_parser_error("%s attribute already defined", name);

	if(attr_init(&attr, name, value) != 0)
		return -1;

	if(vector_add(attrs, &attr) != 0)
		return devtree_parser_error("%s adding attribute failed", name);

	return 0;
}

int attr_add(attr_value_t *value, attr_value_t *op){
	char *s;
	ATTR_INT_TYPE *v;


	// TODO should ilists be allowed to add ints
	if(attr_type_check(value, op->type) != 0)
		return -1;

	switch(value->type){
	case MT_INT:
		value->i += op->i;
		break;

	case MT_ADDR:
		value->p += (ptrdiff_t)op->p;
		break;

	case MT_STRING:
		s = malloc(strlen(value->p) + strlen(op->p) + 1);

		if(s == 0x0)
			return devtree_parser_error("out of memory");

		sprintf(s, "%s%s", value->p, op->p);
		free(value->p);
		value->p = s;
		break;

	case MT_ILIST:
		vector_for_each(&op->v, v){
			if(vector_add(&value->v, v) != 0)
				return devtree_parser_error("adding lists failed");
		}
		break;

	default:
		return devtree_parser_error("addition not supported for type %s", attr_strtype(value->type));
	}

	return 0;
}

int attr_copy(attr_value_t *dest, attr_value_t *src){
	*dest = *src;

	switch(src->type){
	case MT_ILIST:
		if(vector_copy(&dest->v, &src->v) != 0)
			goto err;
		break;

	case MT_STRING:
		dest->p = strdup(src->p);

		if(dest->p == 0x0)
			goto err;
		break;

	default:
		break;
	}

	return 0;


err:
	return devtree_parser_error("attribute copy failed");
}

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef){
	return attr_get_typed(attrs, name, MT_UNDEF, maybe_undef);
}

attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef){
	attr_t *attr;


	vector_for_each(attrs, attr){
		if(strcmp(attr->name, name) == 0){
			if(type == MT_UNDEF || attr_type_check(&attr->value, type) == 0)
				return attr;

			devtree_parser_error("%s: invalid type %s, expecting %s", name, attr_strtype(attr->value.type), attr_strtype(type));

			return 0x0;
		}
	}

	if(!maybe_undef)
		devtree_parser_error("undefined attribute \"%s\"", name);

	return 0x0;
}

int attr_type_check(attr_value_t *value, attr_type_t type){
	if(value->type == MT_UNDEF)
		return devtree_parser_error("attribute with undefined type");

	if(value->type == type)
		return 0;

	if((value->type == MT_INT || value->type == MT_ADDR) && (type == MT_INT || type == MT_ADDR))
		return 0;

	if(value->type != type)
		return devtree_parser_error("type mismatch have %s expected %s", attr_strtype(value->type), attr_strtype(type));

	return 0;
}

int attr_range_check(attr_t *attr, attr_value_t *value){
	ATTR_INT_TYPE lim,
				  *v;


	if(value->type != MT_INT && value->type != MT_ILIST)
		return 0;

	lim = (((ATTR_INT_TYPE)1 << ((value->size * 8)- 1)) << 1) - 1;

	if(value->type == MT_ILIST){
		vector_for_each(&value->v, v){
			if(*v > lim)
				return devtree_parser_error("%s out of range %lu > %lu", attr->name, *v, lim);
		}
	}
	else if(value->i > lim)
		return devtree_parser_error("%s out of range %lu > %lu", attr->name, value->i, lim);

	return 0;
}

char const *attr_strtype(attr_type_t type){
	static char const *names[] = {
		"undef",
		"addr",
		"int",
		"ilist",
		"string",
	};

	if(type < 0 || type > MT_STRING)
		type = MT_UNDEF;

	return names[type];
}

int attr_ilist_add(vector_t *lst, ATTR_INT_TYPE value){
	if(vector_add(lst, &value) != 0)
		return devtree_parser_error("intlist extension failed");

	return 0;
}
