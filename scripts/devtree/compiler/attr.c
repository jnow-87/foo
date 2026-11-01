/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "expr.h"


/* global functions */
int attrvec_init(attrvec_t *attrs, size_t type_size, size_t n){
	return vector_init(attrs, type_size, n);
}

void attrvec_reset(attrvec_t *attrs){
	attr_t *attr;


	// TODO shall the branch be based on the size?
	attrvec_for_each(attrs, attr){
		if(attrs->dt_size == sizeof(type_attr_t) && ((type_attr_t*)attr)->value != 0x0)
			expr_free(((type_attr_t*)attr)->value);

		if(attrs->dt_size == sizeof(node_attr_t))
			expr_value_reset(&((node_attr_t*)attr)->value);
	}

	vector_reset(attrs);
}

int attrvec_add(attrvec_t *attrs, attr_t *attr, size_t attr_size){
	if(attrs->dt_size != attr_size)
		return devtree_parser_error("%s: attribute of invalid type", attr->name);

	if(attrvec_query(attrs, attr->name, true) != 0x0)
		return devtree_parser_error("%s: attribute already defined", attr->name);

	if(vector_add(attrs, attr) != 0)
		return devtree_parser_error("%s: adding attribute failed", attr->name);

	return 0;
}

attr_t *attrvec_query(attrvec_t *attrs, char const *name, bool maybe_undef){
	attr_t *attr;


	attrvec_for_each(attrs, attr){
		if(attr->name != 0x0 && strcmp(attr->name, name) == 0)
			return attr;
	}

	if(!maybe_undef)
		devtree_parser_error("%s: undefined attribute", name);

	return 0x0;
}
