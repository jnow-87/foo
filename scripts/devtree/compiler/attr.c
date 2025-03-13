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
#include <attr.h>
#include <parser.tab.h>


/* types */
typedef int (*op_t)(attr_t *a0, attr_t *a1);


/* local/static prototypes */
static int array_add(attr_t *attr, attr_value_t *v);
static attr_type_t type_compatible(attr_t *a0, attr_t *a1, bool check_array_size, char const *descr);
static attr_type_t type_common(attr_t *a0, attr_t *a1);
static int type_cast(attr_t *attr, attr_type_t type, attr_flags_t flags);
static bool type_is_int(attr_type_t type);

static int range_check(attr_t *attr, attr_value_t *value);

static attr_t *op_wrapper(attr_t *a0, attr_t *a1, op_t op);
static int op_assign(attr_t *a0, attr_t *a1);
static int op_add(attr_t *a0, attr_t *a1);


/* global functions */
attr_t *attr_init(attr_t *attr, char const *name, attr_type_t type, size_t array_limit, attr_value_t *value){
	attr->name = name;
	attr->type = type;
	attr->flags = AF_NONE;

	if(array_limit){
		attr->flags |= AF_ARRAY;
		attr->value.arr.items = VECTOR_INITIALISER(sizeof(attr_value_t));
		attr->value.arr.limit = array_limit;
	}

	if(value == 0x0)
		return attr;

	if(range_check(attr, value) != 0)
		return 0x0;

	attr->value = *value;
	attr->flags |= AF_HAS_VALUE;

	return attr;
}

int attr_enlist(vector_t *attrs, attr_t *attr){
	if(attr->name == 0x0)
		return devtree_parser_error("unable to define unnamed attribute");

	if(attr_get(attrs, attr->name, true) != 0x0)
		return devtree_parser_error("%s: attribute already defined", attr->name);

	if(vector_add(attrs, attr) != 0)
		return devtree_parser_error("%s: adding attribute failed", attr->name);

	return 0;
}

attr_t *attr_assign(attr_t *attr, attr_t *value){
	return op_wrapper(attr, value, op_assign);
}

int attr_add(attr_t *attr, attr_t *op){
	return -(op_wrapper(attr, op, op_add) == 0x0);
}

int attr_copy(attr_t *dest, attr_t *src){
	*dest = *src;

	if(src->flags & AF_ARRAY){
		dest->value.arr.limit = src->value.arr.limit;

		if(vector_copy(&dest->value.arr.items, &src->value.arr.items) != 0)
			goto err;
	}
	else if(src->type == AT_STRING){
		dest->value.p = strdup(src->value.p);

		if(dest->value.p == 0x0)
			goto err;
	}

	return 0;


err:
	return devtree_parser_error("%s: attribute copy failed", src->name);
}

attr_t *attr_get(vector_t *attrs, char const *name, bool maybe_undef){
	return attr_get_typed(attrs, name, AT_UNDEF, maybe_undef);
}

attr_t *attr_get_typed(vector_t *attrs, char const *name, attr_type_t type, bool maybe_undef){
	attr_t *attr;


	vector_for_each(attrs, attr){
		if(strcmp(attr->name, name) == 0){
			if(type == AT_UNDEF || attr->type == type)
				return attr;

			devtree_parser_error("%s: invalid type %s, expecting %s", name, attr_type_name(attr->type), attr_type_name(type));

			return 0x0;
		}
	}

	if(!maybe_undef)
		devtree_parser_error("%s: undefined attribute", name);

	return 0x0;
}

char const *attr_type_name(attr_type_t type){
	static char const *names[] = {
		"undef",
		"int8",
		"int16",
		"int32",
		"int64",
		"addr",
		"string",
	};

	if(type < 0 || type > AT_STRING)
		type = AT_UNDEF;

	return names[type];
}

attr_t *attr_convert_to_list(attr_t *attr){
	attr_value_t v = attr->value;


	attr->flags |= AF_ARRAY;
	attr->value.arr.items = VECTOR_INITIALISER(sizeof(attr_value_t));
	attr->value.arr.limit = ATTR_ARRAY_UNLIMITED;

	array_add(attr, &v);

	return attr;
}

bool type_is_int(attr_type_t type){
	return (type == AT_INT8 || type == AT_INT16 || type == AT_INT32 || type == AT_INT64);
}

size_t attr_type_size(attr_type_t type){
	switch(type){
	case AT_ADDR:	return sizeof(void*);
	case AT_INT8:	return 1;
	case AT_INT16:	return 2;
	case AT_INT32:	return 4;
	case AT_INT64:	return 8;
	default:		return 1;
	}
}


/* local functions */
static int array_add(attr_t *attr, attr_value_t *v){
	if(vector_add(&attr->value.arr.items, v) != 0)
		return devtree_parser_error("%s: adding arrays failed", attr->name);

	attr->value.arr.limit = MAX(attr->value.arr.limit, attr->value.arr.items.size);

	return 0;
}

static attr_type_t type_compatible(attr_t *a0, attr_t *a1, bool check_array_size, char const *descr){
	char const *name = a0->name ? a0->name : (a1->name ? a1->name : descr);
	size_t limit;
	attr_type_t common_type;


	common_type = type_common(a0, a1);

//	if(a0->type == AT_UNDEF){
//		a0->type = a1->type;
//
//		if(a1->flags & AF_ARRAY){
//			a0->flags |= AF_ARRAY;
//			a0->value.arr.limit = ATTR_ARRAY_UNLIMITED;
//		}
//
//		return true;
//	}

	if(common_type != AT_UNDEF && (a0->type == AT_UNDEF || a1->type == AT_UNDEF))
		return common_type;

	if(common_type == AT_UNDEF || (a0->flags & AF_ARRAY) != (a1->flags & AF_ARRAY)){
		devtree_parser_error("%s: incompatible types %s%s and %s%s",
			name,
			attr_type_name(a0->type), (a0->flags & AF_ARRAY) ? " array" : "",
			attr_type_name(a1->type), (a1->flags & AF_ARRAY) ? " array" : ""
		);

		return AT_UNDEF;
	}

	if(check_array_size && (a0->flags & AF_ARRAY)){
		limit = a1->value.arr.limit;

		if(limit == ATTR_ARRAY_UNLIMITED)
			limit = a1->value.arr.items.size;

		if(a0->value.arr.limit != ATTR_ARRAY_UNLIMITED && a0->value.arr.limit != limit){
			devtree_parser_error("%s: incompatible array sizes %zu and %zu",
				name,
				a0->value.arr.limit,
				limit
			);

			return AT_UNDEF;
		}
	}

	return common_type;
}

static attr_type_t type_common(attr_t *a0, attr_t *a1){
	if(a0->type == a1->type || a1->type == AT_UNDEF)
		return a0->type;

	if(a0->type == AT_UNDEF)
		return a1->type;

	if(a0->type == AT_STRING || a1->type == AT_STRING)
		return AT_UNDEF;

	if(type_is_int(a0->type) && type_is_int(a1->type))
		return MAX(a0->type, a1->type);

	return AT_ADDR;
}

static int type_cast(attr_t *attr, attr_type_t type, attr_flags_t flags){
	attr_value_t *v;


	if(attr->type == type)
		return 0;

	if(attr->type == AT_UNDEF){
		// TODO try to combine this branch with the same assingments
		// 		later in the function
		attr->type = type;
		attr->flags = flags;
		attr->value.arr.limit = ATTR_ARRAY_UNLIMITED;

		return 0;
	}

	if(type == AT_UNDEF || type == AT_STRING || attr->type == AT_STRING){
		return devtree_parser_error("%s: unable to cast from %s to %s",
			attr->name,
			attr_type_name(attr->type),
			attr_type_name(type)
		);
	}

	attr->type = type;
	attr->flags = flags;

	// there is no cast needed for different integer types since they all use attr_value_t::i
	if(type_is_int(type))
		return 0;

	if(flags & AF_ARRAY){
		vector_for_each(&attr->value.arr.items, v){
			v->p = (void*)v->i; break;
		}
	}
	else
		attr->value.p = (void*)attr->value.i;

	return 0;
}

static int range_check(attr_t *attr, attr_value_t *value){
	ATTR_INT_TYPE lim;
	attr_value_t *v;


	if(!type_is_int(attr->type))
		return 0;

	lim = (((ATTR_INT_TYPE)1 << ((attr_type_size(attr->type) * 8) - 1)) << 1) - 1;

	if(attr->flags & AF_ARRAY){
		vector_for_each(&value->arr.items, v){
			if(v->i > lim)
				return devtree_parser_error("%s: out of range %lu > %lu", attr->name, v->i, lim);
		}
	}
	else if(value->i > lim)
		return devtree_parser_error("%s: out of range %lu > %lu", attr->name, value->i, lim);

	return 0;
}

static char const *op_name(op_t op){
	if(op == op_assign)	return "'='";
	if(op == op_add)	return "'+'";

	return "unknown";
}

static attr_t *op_wrapper(attr_t *a0, attr_t *a1, op_t op){
	attr_type_t common_type;


	common_type = type_compatible(a0, a1, (op == op_assign), op_name(op));

	if(common_type == AT_UNDEF)
		return 0x0;

	if((op != op_assign || a0->type == AT_UNDEF) && type_cast(a0, common_type, a0->flags | a1->flags) != 0)
		return 0x0;

	if(type_cast(a1, common_type, a0->flags | a1->flags) != 0)
		return 0x0;

	return (op(a0, a1) != 0) ? 0x0 : a0;
}

static int op_assign(attr_t *a0, attr_t *a1){
	attr_value_t v = a1->value;


	if((a0->flags & AF_ARRAY) && a0->value.arr.limit != ATTR_ARRAY_UNLIMITED)
		v.arr.limit = a0->value.arr.limit;

	if(range_check(a0, &v) != 0)
		return -1;

	a0->flags = a1->flags | AF_HAS_VALUE;
	a0->value = v;

	return 0;
}

static int op_add(attr_t *a0, attr_t *a1){
	char *s;
	attr_value_t *v;


	if(a0->flags & AF_ARRAY){
		if(range_check(a0, &a1->value) != 0)
			return -1;

		vector_for_each(&a1->value.arr.items, v){
			if(array_add(a0, v) != 0)
				return -1;
		}

		return 0;
	}

	// TODO should the results be range-checked
	switch(a0->type){
	case AT_INT8:	// fall through
	case AT_INT16:	// fall through
	case AT_INT32:	// fall through
	case AT_INT64:
		a0->value.i += a1->value.i;
		break;

	case AT_ADDR:
		a0->value.p += (ptrdiff_t)a1->value.p;
		break;

	case AT_STRING:
		if(a0->value.p == 0x0 || a1->value.p == 0x0)
			return devtree_parser_error("null pointer strings in %s or %s", a0->name, a1->name);

		s = malloc(strlen(a0->value.p) + strlen(a1->value.p) + 1);

		if(s == 0x0)
			return devtree_parser_error("out of memory");

		sprintf(s, "%s%s", a0->value.p, a1->value.p);
		free(a0->value.p);
		a0->value.p = s;
		break;

	default:
		return devtree_parser_error("addition not supported for types %s and %s",
			attr_type_name(a0->type),
			attr_type_name(a1->type)
		);
	}

	return 0;
}
