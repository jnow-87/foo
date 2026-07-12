/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/math.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "expr.h"


/* local/static prototypes */
static expr_arg_t *resolve_ref(expr_arg_t *arg, void *ctx, expr_arg_t *res);

//static attr_type_t types_compatible(attr_t *a0, attr_t *a1, bool check_array_size, char const *descr);
//static attr_type_t type_common(attr_t *a0, attr_t *a1);
//static int type_cast(attr_t *attr, attr_type_t type, attr_flags_t flags);
//static bool type_is_int(attr_type_t type);
//
//static int range_check(attr_t *attr, attr_value_t *value);
//static char const *op_name(attr_op_t op);
//static int math_valid(attr_t *a0, attr_t *a1, attr_op_t op);



// TODO
// 	- make expr_create() to evaluate an expression of args are literals
// 	- add attr type "reference"
// 	- update expr_evaluate() to throw an error if a reference cannot be resolved

/* global functions */
expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1){
	// TODO consider who should free arg0 and arg1 if both are literals
	// TODO check if both args are literals, if so, evaluate the expression instead
	if(arg0->op == expr_literal && arg1->op == expr_literal){
		op(arg0->arg0, arg1->arg0, expr->arg0);
		expr_destroy(arg1);

		return expr;
	}

	expr = malloc(sizeof(expr_t));

	if(expr == 0x0)
		return 0x0;

	expr->op = op;
	expr->arg0->type = ET_UNDEF;
//	expr->arg1 = *arg1;

	return expr;
}

expr_t *expr_alloc(expr_t *expr){
	return expr;
}

void expr_destroy(expr_t *expr){
	free(expr);
}

size_t expr_type_size(expr_type_t type){
	return 0;
}

int expr_type_check(expr_t *expr, expr_type_t type){
//	if(range_check(attr, &v) != 0)
//		return -1;
//
	return 0;
}

char const *expr_type_name(expr_type_t type){
	return "";
}

int expr_array_add(expr_t *array, expr_t *expr){
//	if(!array->arg0->is_array)
//		return devtree_parser_error("appending to something not an array");
//
//	if(vector_add(&array->arg0->value.array.items, &(expr_value_t){ .expr = *expr }) != 0)
//		return devtree_parser_error("adding to array failed");
//
//	array->arg0->value.array.limit = MAX(array->value.array.limit, array->value.array.items.size);

	return 0;
}

int expr_copy(expr_t *dest, expr_t *src){
	*dest = *src;

/* TODO impl, cf. attr_copy()
	if(src->flags & AF_ARRAY){
		dest->value.arr.limit = src->value.arr.limit;

		if(vector_copy(&dest->value.arr.items, &src->value.arr.items) != 0)
			goto err;
	}
	else if(src->type == ET_STRING){
		dest->value.p = strdup(src->value.p);

		if(dest->value.p == 0x0)
			goto err;
	}
*/
	return 0;

/*
err:
	return devtree_parser_error("%s: attribute copy failed", src->name);
*/}

expr_arg_t *expr_evaluate(expr_t *expr, expr_arg_t *result, void *ctx){
	expr_arg_t arg0,
			   arg1;


	if(expr->op == expr_literal)
		return expr_literal(expr->arg0, 0x0, result);

	if(expr->op == expr_reference)
		return resolve_ref(expr->arg0, ctx, result);

	expr_evaluate(expr, &arg0, ctx);
	expr_evaluate(expr, &arg1, ctx);

	return expr->op(&arg0, &arg1, result);
}

expr_arg_t *expr_literal(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	*res = *arg0;

	return res;
}

expr_arg_t *expr_reference(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return 0x0;
}

expr_arg_t *expr_add(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_sub(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_mul(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_div(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_lshift(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_rshift(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_mod(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_eq(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_neq(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_lesser(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_lesser_eq(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_greater(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_greater_eq(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_bit_and(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_bit_or(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_bit_xor(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_log_and(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}

expr_arg_t *expr_log_or(expr_arg_t *arg0, expr_arg_t *arg1, expr_arg_t *res){
	return res;
}


/* local functions */
static expr_arg_t *resolve_ref(expr_arg_t *arg, void *ctx, expr_arg_t *res){
	attr_t *ref;


	ref = attr_query(ctx, arg->value.p, false);

	if(ref == 0x0)
		return 0x0;

	*res = *ref->value->arg0;

	return res;
}

//attr_t *attr_math(attr_t *a0, attr_t *a1, attr_op_t op, bool resolve){
//	char *s;
//	attr_value_t *v;
//
//
//	if(math_valid(a0, a1, op) != 0)
//		return 0x0;
//
//	if(!resolve){
//		// TODO create
//	}
//
//	if(a0->flags & AF_ARRAY){
//		vector_for_each(&a1->value.arr.items, v){
//			if(array_add(a0, v) != 0)
//				return 0x0;
//		}
//
//		return a0;
//	}
//
//	// TODO should the results be range-checked
//	switch(a0->type){
//	case ET_INT8:	// fall through
//	case ET_INT16:	// fall through
//	case ET_INT32:	// fall through
//	case ET_INT64:
//		switch(op){
//		case OP_ADD:	a0->value.i += a1->value.i; break;
//		case OP_SUB:	a0->value.i -= a1->value.i; break;
//		case OP_MUL:	a0->value.i *= a1->value.i; break;
//		case OP_DIV:	a0->value.i /= a1->value.i; break;
//		case OP_LSHIFT:	a0->value.i <<= a1->value.i; break;
//		case OP_RSHIFT:	a0->value.i >>= a1->value.i; break;
//		case OP_MOD:	a0->value.i %= a1->value.i; break;
//		default:		break;
//		}
//
//		break;
//
//	case ET_ADDR:
//		a0->value.p += (ptrdiff_t)a1->value.p;
//		break;
//
//	case ET_STRING:
//		s = malloc(strlen(a0->value.p) + strlen(a1->value.p) + 1);
//
//		if(s == 0x0){
//			devtree_parser_error("out of memory");
//			return 0x0;
//		}
//
//		sprintf(s, "%s%s", a0->value.p, a1->value.p);
//		free(a0->value.p);
//		a0->value.p = s;
//		break;
//
//	default:
//		return 0x0;
//	}
//
//	return a0;
//}
//
//bool type_is_int(attr_type_t type){
//	return (type == ET_INT8 || type == ET_INT16 || type == ET_INT32 || type == ET_INT64);
//}
//
//size_t attr_type_size(attr_type_t type){
//	switch(type){
//	case ET_ADDR:	return sizeof(void*);
//	case ET_INT8:	return 1;
//	case ET_INT16:	return 2;
//	case ET_INT32:	return 4;
//	case ET_INT64:	return 8;
//	default:		return 1;
//	}
//}
//
//static int array_add(attr_t *attr, attr_value_t *v){
//	if(vector_add(&attr->value.arr.items, v) != 0)
//		return devtree_parser_error("%s: adding arrays failed", attr->name);
//
//	attr->value.arr.limit = MAX(attr->value.arr.limit, attr->value.arr.items.size);
//
//	return 0;
//}
//
//static attr_type_t types_compatible(attr_t *a0, attr_t *a1, bool check_array_size, char const *descr){
//	char const *name = a0->name ? a0->name : (a1->name ? a1->name : descr);
//	size_t limit;
//	attr_type_t common_type;
//
//
//	common_type = type_common(a0, a1);
//
///*
//	if(a0->type == ET_UNDEF){
//		a0->type = a1->type;
//
//		if(a1->flags & AF_ARRAY){
//			a0->flags |= AF_ARRAY;
//			a0->value.arr.limit = ATTR_ARRAY_UNLIMITED;
//		}
//
//		return true;
//	}
//*/
//
//	if(common_type != ET_UNDEF && (a0->type == ET_UNDEF || a1->type == ET_UNDEF))
//		return common_type;
//
//	if(common_type == ET_UNDEF || (a0->flags & AF_ARRAY) != (a1->flags & AF_ARRAY)){
//		devtree_parser_error("%s: incompatible types %s%s and %s%s",
//			name,
//			expr_type_name(a0->type), (a0->flags & AF_ARRAY) ? " array" : "",
//			expr_type_name(a1->type), (a1->flags & AF_ARRAY) ? " array" : ""
//		);
//
//		return ET_UNDEF;
//	}
//
//	if(check_array_size && (a0->flags & AF_ARRAY)){
//		limit = a1->value.arr.limit;
//
//		if(limit == ATTR_ARRAY_UNLIMITED)
//			limit = a1->value.arr.items.size;
//
//		if(a0->value.arr.limit != ATTR_ARRAY_UNLIMITED && a0->value.arr.limit != limit){
//			devtree_parser_error("%s: incompatible array sizes %zu and %zu",
//				name,
//				a0->value.arr.limit,
//				limit
//			);
//
//			return ET_UNDEF;
//		}
//	}
//
//	return common_type;
//}
//
//static attr_type_t type_common(attr_t *a0, attr_t *a1){
//	if(a0->type == a1->type || a1->type == ET_UNDEF)
//		return a0->type;
//
//	if(a0->type == ET_UNDEF)
//		return a1->type;
//
//	if(a0->type == ET_STRING || a1->type == ET_STRING)
//		return ET_UNDEF;
//
//	if(type_is_int(a0->type) && type_is_int(a1->type))
//		return MAX(a0->type, a1->type);
//
//	return ET_ADDR;
//}
//
//static int type_cast(attr_t *attr, attr_type_t type, attr_flags_t flags){
//	attr_value_t *v;
//
//
//	if(attr->type == ET_UNDEF){
//		attr->value.arr.limit = ATTR_ARRAY_UNLIMITED;
//		attr->type = type;
//		attr->flags = flags;
//	}
//
//	if(attr->type == type)
//		return 0;
//
//	if(type == ET_UNDEF || type == ET_STRING || attr->type == ET_STRING){
//		return devtree_parser_error("%s: unable to cast from %s to %s",
//			attr->name,
//			expr_type_name(attr->type),
//			expr_type_name(type)
//		);
//	}
//
//	attr->type = type;
//	attr->flags = flags;
//
//	// there is no cast needed for different integer types since they all use attr_value_t::i
//	if(type_is_int(type))
//		return 0;
//
//	if(flags & AF_ARRAY){
//		vector_for_each(&attr->value.arr.items, v){
//			v->p = (void*)v->i; break;
//		}
//	}
//	else
//		attr->value.p = (void*)attr->value.i;
//
//	return 0;
//}
//
//static int range_check(attr_t *attr, attr_value_t *value){
//	ATTR_INT_TYPE lim;
//	attr_value_t *v;
//
//
//	if(!type_is_int(attr->type))
//		return 0;
//
//	lim = (((ATTR_INT_TYPE)1 << ((attr_type_size(attr->type) * 8) - 1)) << 1) - 1;
//
//	if(attr->flags & AF_ARRAY){
//		vector_for_each(&value->arr.items, v){
//			if(v->i > lim)
//				return devtree_parser_error("%s: out of range %lu > %lu", attr->name, v->i, lim);
//		}
//	}
//	else if(value->i > lim)
//		return devtree_parser_error("%s: out of range %lu > %lu", attr->name, value->i, lim);
//
//	return 0;
//}
//
//static char const *op_name(attr_op_t op){
//	switch(op){
//	case OP_ADD:	return "'+'";
//	case OP_SUB:	return "'-'";
//	case OP_MUL:	return "'*'";
//	case OP_DIV:	return "'/'";
//	case OP_LSHIFT:	return "'<<'";
//	case OP_RSHIFT:	return "'>>'";
//	case OP_MOD:	return "'%'";
//	default:		return "unknown";
//	}
//}
//
//static int types_align(attr_t *a0, attr_t *a1, bool is_assignment, char const *descr){
//	attr_type_t common_type;
//
//
//	common_type = types_compatible(a0, a1, is_assignment, descr);
//
//	if(common_type == ET_UNDEF)
//		return -1;
//
//	if(a0->flags & AF_ARRAY){
//		if(range_check(a0, &a1->value) != 0)
//			return -1;
//	}
//
//	if((!is_assignment || a0->type == ET_UNDEF) && type_cast(a0, common_type, a0->flags | a1->flags) != 0)
//		return -1;
//
//	if(type_cast(a1, common_type, a0->flags | a1->flags) != 0)
//		return -1;
//
//	return 0;
//}
//
//static int math_valid(attr_t *a0, attr_t *a1, attr_op_t op){
//	// TODO this function shouldn't change anything in the arguments
//	if(types_align(a0, a1, false, op_name(op)) != 0)
//		return -1;
//
//	if(a0->flags & AF_ARRAY)
//		return (op != OP_ADD) ? devtree_parser_error("%s not supported for arrays", op_name(op)) : 0;
//
//	switch(a0->type){
//	case ET_INT8:	// fall through
//	case ET_INT16:	// fall through
//	case ET_INT32:	// fall through
//	case ET_INT64:
//		return 0;
//
//	case ET_ADDR:
//		if(op != OP_ADD)
//			return devtree_parser_error("foo not supported for pointer types");
//
//		return 0;
//
//	case ET_STRING:
//		if(op != OP_ADD)
//			return devtree_parser_error("foo not supported for string types");
//
//		if(a0->value.p == 0x0 || a1->value.p == 0x0)
//			return devtree_parser_error("null pointer strings in %s or %s", a0->name, a1->name);
//
//		return 0;
//
//	default:
//		return devtree_parser_error("%s not supported for types %s and %s",
//			op_name(op),
//			expr_type_name(a0->type),
//			expr_type_name(a1->type)
//		);
//	}
//}
//
//char const *expr_type_name(expr_type_t type){
//	static char const *names[] = {
//		"undef",
//		"int8",
//		"int16",
//		"int32",
//		"int64",
//		"addr",
//		"string",
//	};
//
//	if(type < 0 || type > ET_STRING)
//		type = ET_UNDEF;
//
//	return names[type];
//}
