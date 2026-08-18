/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "attrvec.h"
#include "expr.h"


/* global functions */
int attrvec_init(attrvec_t *attrs, size_t n){
	return vector_init(attrs, sizeof(attr_t), n);
}

void attrvec_destroy(attrvec_t *attrs){
	vector_destroy(attrs);
}

int attrvec_add(attrvec_t *attrs, attr_t *attr){
	if(attrvec_query(attrs, attr->name, true) != 0x0)
		return devtree_parser_error("%s: attribute already defined", attr->name);

	if(vector_add(attrs, attr) != 0)
		return devtree_parser_error("%s: adding attribute failed", attr->name);

	return 0;
}

attr_t *attrvec_query(attrvec_t *attrs, char const *name, bool maybe_undef){
	return attrvec_query_typed(attrs, name, ET_UNDEF, maybe_undef);
}

attr_t *attrvec_query_typed(attrvec_t *attrs, char const *name, expr_type_t type, bool maybe_undef){
	attr_t *attr;


	attrvec_for_each(attrs, attr){
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
