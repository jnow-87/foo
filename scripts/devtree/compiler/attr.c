/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/math.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <attr.h>
#include <parser.tab.h>


#define ATTR_PRINT(txt, attr){ \
	printf(txt); \
	attr_print(attr); \
	printf("\n"); \
}


/* local/static prototypes */
static int array_add(attr_t *attr, attr_value_t *v);
static void attr_print(attr_t *attr);
static void attr_value_print(attr_value_t *v, attr_type_t type);
static bool types_compatible(attr_t *attr0, attr_t *attr1);
static int range_check(attr_t *attr, attr_value_t *value);


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

	// TODO handle array case
	attr->value = *value;
	attr->flags |= AF_HAS_VALUE;

	return attr;
}

int attr_enlist(vector_t *attrs, attr_t *attr){
	if(attr->name == 0x0)
		return devtree_parser_error("unable to define unnamed attribute");

	if(attr_get(attrs, attr->name, true) != 0x0)
		return devtree_parser_error("%s attribute already defined", attr->name);

	if(vector_add(attrs, attr) != 0)
		return devtree_parser_error("%s adding attribute failed", attr->name);

	return 0;
}

attr_t *attr_assign(attr_t *attr, attr_t *value){
	attr_value_t v;


	ATTR_PRINT("assign ", attr);
	ATTR_PRINT("  with ", value);

	if(!types_compatible(attr, value))
		return 0x0;

	if((attr->flags & AF_ARRAY) != (value->flags & AF_ARRAY)){
		devtree_parser_error("cannot assign %s to %s",
			(value->flags & AF_ARRAY) ? "array" : "single value",
			(attr->flags & AF_ARRAY) ? "array" : "single value"
		);

		return 0x0;
	}

	v = value->value;

	if((attr->flags & AF_ARRAY) && attr->value.arr.limit != ATTR_ARRAY_UNLIMITED)
		v.arr.limit = attr->value.arr.limit;

	if(range_check(attr, &value->value) != 0)
		return 0x0;

	attr->flags = value->flags | AF_HAS_VALUE;
	attr->value = v;

	ATTR_PRINT("assigned ", attr);

	return attr;
}

int attr_add(attr_t *attr, attr_t *op){
	char *s;
	attr_value_t *v;


	if(!types_compatible(attr, op))
		goto err;

	ATTR_PRINT("add ", op)
	ATTR_PRINT(" to ", attr);

	if(attr->flags & AF_ARRAY){
		/* TODO only arrays can be added, no longer single values, so remove */
		if(op->flags & AF_ARRAY){
			if(range_check(attr, &op->value) != 0)
				return -1;

			vector_for_each(&op->value.arr.items, v){
				if(array_add(attr, v) != 0)
					return -1;
			}
		}
		else
			return array_add(attr, &op->value);

		return 0;
	}

	// TODO should the results be range-checked
	switch(attr->type){
	case AT_INT8:	// fall through
	case AT_INT16:	// fall through
	case AT_INT32:	// fall through
	case AT_INT64:
		attr->value.i += op->value.i;
		break;

	case AT_ADDR:
		if(op->type == AT_ADDR)	attr->value.p += (ptrdiff_t)op->value.p;
		else					attr->value.p += op->value.i;
		break;

	case AT_STRING:
		if(attr->value.p == 0x0)
			return devtree_parser_error("attr value 0x0");

		s = malloc(strlen(attr->value.p) + strlen(op->value.p) + 1);

		if(s == 0x0)
			return devtree_parser_error("out of memory");

		sprintf(s, "%s%s", attr->value.p, op->value.p);
		free(attr->value.p);
		attr->value.p = s;
		break;

	default:
		goto err;
	}

	return 0;


err:
	return devtree_parser_error("addition not supported for types %s and %s", attr_type_name(attr->type), attr_type_name(op->type));
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
	return devtree_parser_error("attribute copy failed");
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
		devtree_parser_error("undefined attribute \"%s\"", name);

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

bool attr_is_int(attr_t *attr){
	attr_type_t type = attr->type;


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
		return devtree_parser_error("adding arrays failed");

	printf("added array: %p =", attr->value.arr.items.buf);

	attr_value_t *x;

	printf("[");

	vector_for_each(&attr->value.arr.items, x){
		attr_value_print(x, attr->type);
		printf(" ");
	}

	printf("]\n");

	attr->value.arr.limit = MAX(attr->value.arr.limit, attr->value.arr.items.size);

	return 0;
}


static void attr_print(attr_t *attr){
	attr_value_t *v;


	printf("%s", attr_type_name(attr->type));

	if(attr->flags & AF_ARRAY){
		printf("[limit=");

		if(attr->value.arr.limit == ATTR_ARRAY_UNLIMITED)
			printf("unlimited");
		else
			printf("%zu", attr->value.arr.limit);

		printf(",size=%zu", attr->value.arr.items.size);

		printf("]");
	}

	printf(" %s ", attr->name ? attr->name : "unnamed");

	if((attr->flags & AF_HAS_VALUE) == 0)
		return;

	printf("=");

	if(attr->flags & AF_ARRAY){
		printf("[");

		vector_for_each(&attr->value.arr.items, v)
			attr_value_print(v, attr->type);

		printf("]");
	}
	else
		attr_value_print(&attr->value, attr->type);
}

static void attr_value_print(attr_value_t *v, attr_type_t type){
	switch(type){
	case AT_INT8:
	case AT_INT16:
	case AT_INT32:
	case AT_INT64:
		printf("%zu", v->i); break;

	case AT_ADDR:
		printf("%p", v->p); break;

	case AT_STRING:
		printf("\"%s\"", v->p); break;

	default:
		printf("undef"); break;
	}
}

static bool types_compatible(attr_t *attr0, attr_t *attr1){
	size_t limit;


	if(attr0->type == AT_UNDEF){
		attr0->type = attr1->type;

		if(attr1->flags & AF_ARRAY){
			attr0->flags |= AF_ARRAY;
			attr0->value.arr.limit = ATTR_ARRAY_UNLIMITED;
		}

		return true;
	}

	if((attr0->flags & AF_ARRAY) != (attr1->flags & AF_ARRAY)){
		devtree_parser_error("incompatible types %s %s and %s %s",
			attr_type_name(attr0->type), (attr0->flags & AF_ARRAY) ? "array" : "value",
			attr_type_name(attr1->type), (attr1->flags & AF_ARRAY) ? "array" : "value"
		);

		return false;
	}

	if((attr0->flags & AF_ARRAY) && (attr1->flags & AF_ARRAY)){
		limit = attr1->value.arr.limit;

		if(limit == ATTR_ARRAY_UNLIMITED)
			limit = attr1->value.arr.items.size;

		if(attr0->value.arr.limit != ATTR_ARRAY_UNLIMITED && attr0->value.arr.limit != limit){
			devtree_parser_error("incompatible array sizes %zu and %zu",
				attr0->value.arr.limit,
				limit
			);

			return false;
		}
	}

	if(attr0->type == attr1->type || (attr_is_int(attr0) && attr_is_int(attr1)))
		return true;

	if(attr0->type == AT_ADDR && attr_is_int(attr1))
		return true;

	devtree_parser_error("incompatible types %s and %s", attr_type_name(attr0->type), attr_type_name(attr1->type));

	return false;
}

static int range_check(attr_t *attr, attr_value_t *value){
	ATTR_INT_TYPE lim;
	attr_value_t *v;


	if(!attr_is_int(attr))
		return 0;

	lim = (((ATTR_INT_TYPE)1 << ((attr_type_size(attr->type) * 8) - 1)) << 1) - 1;

	if(attr->flags & AF_ARRAY){
		printf("range check: %p\n", value->arr.items.buf);

		vector_for_each(&value->arr.items, v){
			if(v->i > lim)
				return devtree_parser_error("%s out of range %lu > %lu", attr->name, v->i, lim);
		}
	}
	else if(value->i > lim)
		return devtree_parser_error("%s out of range %lu > %lu", attr->name, value->i, lim);

	return 0;
}
