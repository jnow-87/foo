/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <stdio.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "expr.h"
#include "node.h"
#include "strmap.h"


/* macros */
#define VALID_ADDR_OPS	(EOP_SUBTRACT | EOP_ADD | EOP_EQUAL | EOP_UNEQUAL | EOP_LESSER | EOP_LESSER_EQUAL | EOP_GREATER | EOP_GREATER_EQUAL)

#define INT_OP(a0, a1, r, op){ \
	r->i = a0->i op a1->i; \
	\
	if(range_check(r, r->type) != 0) \
		return 0x0; \
}


/* types */
typedef expr_value_t *(*op_cb_t)(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);


/* local/static prototypes */
static int resolve_ref(expr_value_t *arg, node_attrvec_t *ctx, expr_value_t *res);

static int types_compatible(expr_type_t t0, expr_type_t t1, bool t0_is_array, bool t1_is_array);
static int op_defined(expr_op_t op, expr_value_t *v0, expr_value_t *v1);
static int range_check(expr_value_t *value, expr_type_t type);

static expr_value_t *op_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_subtract(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_multiply(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_divide(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_left_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_right_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_modulo(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_unequal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_xor(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_log_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_log_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);

static array_t *add_array(expr_value_t *arr, expr_value_t *arg);
static char const *add_str(char const *s0, char const *s1);


/* static variables */
static op_cb_t ops[] = {
	[EOP_LITERAL] = 0x0,
	[EOP_REFERENCE] = 0x0,
	[EOP_ADD] = op_add,
	[EOP_SUBTRACT] = op_subtract,
	[EOP_MULTIPLY] = op_multiply,
	[EOP_DIVIDE] = op_divide,
	[EOP_LEFT_SHIFT] = op_left_shift,
	[EOP_RIGHT_SHIFT] = op_right_shift,
	[EOP_MODULO] = op_modulo,
	[EOP_EQUAL] = op_equal,
	[EOP_UNEQUAL] = op_unequal,
	[EOP_LESSER] = op_lesser,
	[EOP_LESSER_EQUAL] = op_lesser_eq,
	[EOP_GREATER] = op_greater,
	[EOP_GREATER_EQUAL] = op_greater_eq,
	[EOP_BIT_AND] = op_bit_and,
	[EOP_BIT_OR] = op_bit_or,
	[EOP_BIT_XOR] = op_bit_xor,
	[EOP_LOG_AND] = op_log_and,
	[EOP_LOG_OR] = op_log_or,
};

static char const *op_name[] = {
	[EOP_LITERAL] = "literal",
	[EOP_REFERENCE] = "reference",
	[EOP_ADD] = "add",
	[EOP_SUBTRACT] = "subtract",
	[EOP_MULTIPLY] = "multiply",
	[EOP_DIVIDE] = "divide",
	[EOP_LEFT_SHIFT] = "left_shift",
	[EOP_RIGHT_SHIFT] = "right_shift",
	[EOP_MODULO] = "modulo",
	[EOP_EQUAL] = "equal",
	[EOP_UNEQUAL] = "unequal",
	[EOP_LESSER] = "lesser",
	[EOP_LESSER_EQUAL] = "lesser equal",
	[EOP_GREATER] = "greater",
	[EOP_GREATER_EQUAL] = "greater equal",
	[EOP_BIT_AND] = "bit_and",
	[EOP_BIT_OR] = "bit_or",
	[EOP_BIT_XOR] = "bit_xor",
	[EOP_LOG_AND] = "log_and",
	[EOP_LOG_OR] = "log_or",
};


/* global functions */
expr_t *expr_alloc(expr_t *expr){
	expr_t *e;


	e = malloc(sizeof(expr_t));

	if(e == 0x0){
		devtree_parser_error("error allocating expression");

		return 0x0;
	}

	if(expr != 0x0)
		*e = *expr;

	return e;
}

void expr_reset(expr_t *expr){
	expr_value_reset(&expr->literal);
	expr_free(expr->arg0);
	expr_free(expr->arg1);

	memset(expr, 0, sizeof(expr_t));
}

void expr_free(expr_t *expr){
	if(expr == 0x0)
		return;

	expr_reset(expr);
	free(expr);
}

expr_value_t *expr_value_mv(expr_value_t *dest, expr_value_t *src){
	*dest = *src;

	src->type = ET_UNDEF;
	src->is_array = false;

	return dest;
}

void expr_value_reset(expr_value_t *value){
	expr_value_t *el;


	if(value->type != ET_UNDEF && value->is_array && !value->array->cached){
		vector_for_each(&value->array->items, el)
			expr_value_reset(el);

		array_free(value->array);
	}

	value->type = ET_UNDEF;
	value->is_array = false;
}

int expr_evaluate(expr_t *expr, expr_value_t *result, struct node_attrvec_t *ctx){
	int r = 0;
	expr_value_t arg0,
				 arg1;


	if(expr->op == EOP_LITERAL){
		memcpy(result, &expr->literal, sizeof(expr_value_t));

		return 0;
	}

	if(expr->op == EOP_REFERENCE)
		return (resolve_ref(&expr->literal, ctx, result) != 0) ? 1 : 0;

	r |= expr_evaluate(expr->arg0, &arg0, ctx);
	r |= expr_evaluate(expr->arg1, &arg1, ctx);

	if(r != 0)
		return r;

	if(types_compatible(arg0.type, arg1.type, arg0.is_array, arg1.is_array) != 0)
		return -1;

	if(op_defined(expr->op, &arg0, &arg1) != 0)
		return -1;

	result->type = expr_type(expr);
	result->is_array = arg0.is_array || arg1.is_array;

	if(result->type & ET_UNDEF)
		return devtree_parser_error("invalid expression result type %s", expr_type_name(result->type));

	if(ops[expr->op](&arg0, &arg1, result) == 0x0)
		return devtree_parser_error("operation %s failed", op_name[expr->op]);

	expr_value_reset(&arg0);
	expr_value_reset(&arg1);

	return 0;
}

expr_type_t expr_type(expr_t *expr){
	expr_type_t t0,
				t1;


	if(expr->op == EOP_LITERAL)
		return expr->literal.type;

	if(expr->op == EOP_REFERENCE)
		return ET_UNDEF;

	if(expr->op & (EOP_EQUAL | EOP_UNEQUAL | EOP_LESSER | EOP_LESSER_EQUAL | EOP_GREATER | EOP_GREATER_EQUAL | EOP_LOG_AND | EOP_LOG_OR))
		return ET_INT8;

	t0 = expr_type(expr->arg0);
	t1 = expr_type(expr->arg1);

	if(t0 == t1 || t1 == ET_UNDEF)
		return t0;

	if(t0 == ET_UNDEF)
		return t1;

	if(t0 == ET_STRING || t1 == ET_STRING)
		return ET_UNDEF;

	if(EXPR_TYPE_IS_INT(t0) && EXPR_TYPE_IS_INT(t1))
		return MAX(t0, t1);

	return ET_ADDR;
}

int expr_type_check(expr_t *expr, expr_type_t type, bool is_array, size_t array_limit){
	bool expr_is_array = (expr->op == EOP_LITERAL && expr->literal.is_array);
	array_t *arr;


	if(types_compatible(expr_type(expr), type, expr_is_array, is_array))
		return -1;

	if(is_array){
		if(!expr_is_array)
			return devtree_parser_error("mixing array and non-array types");

		arr = expr->literal.array;

		if(array_limit != arr->items.size)
			return devtree_parser_error("incompatible array sizes %zu and %zu", array_limit, arr->items.size);
	}

	if(expr->op == EOP_LITERAL)
		return range_check(&expr->literal, type);

	return 0;
}

size_t expr_type_size(expr_type_t type){
	switch(type){
	case ET_INT8:	return 1;
	case ET_INT16:	return 2;
	case ET_INT32:	return 4;
	case ET_INT64:	return 8;
	case ET_ADDR:	return sizeof(void*);
	case ET_STRING:	return sizeof(void*);
	default:		return 0;
	}
}

char const *expr_type_name(expr_type_t type){
	switch(type){
	case ET_INT8:	return "int8";
	case ET_INT16:	return "int16";
	case ET_INT32:	return "int32";
	case ET_INT64:	return "int64";
	case ET_ADDR:	return "addr";
	case ET_STRING:	return "string";
	default:		return "undef";
	}
}

int expr_array_add(expr_t *array, expr_t *expr){
	array_t *arr = array->literal.array;
	expr_type_t etype = expr_type(expr);


	if(expr->op != EOP_LITERAL)
		return devtree_parser_error("array elements have to be literals");

	if(expr->literal.is_array)
		return devtree_parser_error("nested arrays are not supported");

	if(array->op != EOP_LITERAL || !array->literal.is_array)
		return devtree_parser_error("appending to something not an array");

	if(array->literal.type == ET_UNDEF)
		array->literal.type = etype;

	if(types_compatible(array->literal.type, etype, true, expr->literal.is_array) != 0)
		return -1;

	if(array_add(&arr, &expr->literal) == 0x0)
		return devtree_parser_error("adding to array failed");

	return 0;
}


/* local functions */
static int resolve_ref(expr_value_t *arg, node_attrvec_t *ctx, expr_value_t *res){
	node_attr_t *ref;


	if(ctx == 0x0)
		return -1;

	ref = (node_attr_t*)attrvec_query(&ctx->v, arg->p, false);

	if(ref == 0x0)
		return -1;

	*res = ref->value;

	return 0;
}

static int types_compatible(expr_type_t t0, expr_type_t t1, bool t0_is_array, bool t1_is_array){
	uint8_t ints = EXPR_TYPE_IS_INT(t0) + EXPR_TYPE_IS_INT(t1),
			addrs = (t0 == ET_ADDR) + (t1 == ET_ADDR),
			undefs = (t0 == ET_UNDEF) + (t1 == ET_UNDEF);


	if((undefs == 0) && ((t0 == t1) || (ints + addrs == 2)))
		return 0;

	return devtree_parser_error("incompatible types %s%s and %s%s"
		, expr_type_name(t0)
		, t0_is_array ? " array" : ""
		, expr_type_name(t1)
		, t1_is_array ? " array" : ""
	);
}

static int op_defined(expr_op_t op, expr_value_t *v0, expr_value_t *v1){
	uint8_t	addrs = (v0->type == ET_ADDR) + (v1->type == ET_ADDR),
			strs = (v0->type == ET_STRING) + (v1->type == ET_STRING),
			undefs = (v0->type == ET_UNDEF) + (v1->type == ET_UNDEF);


	// all values need to have a type
	if(undefs != 0)
		goto err;

	// strings are only compatible with strings and only via addition
	if(strs == 1 || (strs != 0 && op != EOP_ADD))
		goto err;

	// address must not be added or subtracted
	if(addrs == 2 && ((op & (EOP_ADD | EOP_SUBTRACT)) != 0))
		goto err;

	// invalid operations on addresses
	if(addrs != 0 && ((op & VALID_ADDR_OPS) == 0))
		goto err;

	return 0;


err:
	return devtree_parser_error("undefined operation %s for types %s%s and %s%s"
		, op_name[op]
		, expr_type_name(v0->type)
		, v0->is_array ? " array" : ""
		, expr_type_name(v1->type)
		, v1->is_array ? " array" : ""
	);
}

static int range_check(expr_value_t *value, expr_type_t type){
	expr_value_t *e = value;
	EXPR_INT_T lim;


	if(!EXPR_TYPE_IS_INT(type))
		return 0;

	lim = (((EXPR_INT_T)1 << ((expr_type_size(type) * 8) - 1)) << 1) - 1;

	if(value->is_array){
		vector_for_each(&value->array->items, e){
			if(e->i > lim)
				goto err;
		}
	}
	else if(value->i > lim)
		goto err;

	return 0;


err:
	return devtree_parser_error("integer %lu out of %s range (limit=%lu)", e->i, expr_type_name(type), lim);
}

static expr_value_t *op_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	uint8_t ints = EXPR_TYPE_IS_INT(arg0->type) + EXPR_TYPE_IS_INT(arg1->type),
			addrs = (arg0->type == ET_ADDR) + (arg1->type == ET_ADDR);


	if(arg0->is_array || arg1->is_array){
		res->is_array = true;
		res->array = arg0->is_array ? add_array(arg0, arg1) : add_array(arg1, arg0);

		if(res->array == 0x0)
			return 0x0;
	}
	else if(arg0->type == ET_STRING){
		res->p = add_str(arg0->p, arg1->p);

		if(res->p == 0x0)
			return 0x0;
	}
	else if(ints == 1 && addrs == 1){
		if(arg0->type == ET_ADDR)	res->p = arg0->p + arg1->i;
		else						res->p = arg0->i + arg1->p;
	}
	else if(ints == 2){
		INT_OP(arg0, arg1, res, +);
	}
	else
		return 0x0;

	return res;
}

static expr_value_t *op_subtract(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	if(EXPR_TYPE_IS_INT(arg0->type) && arg1->type == ET_ADDR){
		devtree_parser_error("undefined operation %s for types %s and %s"
			, op_name[EOP_SUBTRACT]
			, expr_type_name(arg0->type)
			, expr_type_name(arg1->type)
		);

		return 0x0;
	}


	if(EXPR_TYPE_IS_INT(arg0->type))	INT_OP(arg0, arg1, res, -)
	else if(arg1->type == ET_ADDR)		res->p = arg0->p - (ptrdiff_t)arg1->p;
	else								res->p = arg0->p - arg1->i;

	return res;
}

static expr_value_t *op_multiply(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, *);

	return res;
}

static expr_value_t *op_divide(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, /);

	return res;
}

static expr_value_t *op_left_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, <<);

	return res;
}

static expr_value_t *op_right_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, >>);

	return res;
}

static expr_value_t *op_modulo(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, %);

	return res;
}

static expr_value_t *op_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i == arg1->i);

	return res;
}

static expr_value_t *op_unequal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i != arg1->i);

	return res;
}

static expr_value_t *op_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i < arg1->i);

	return res;
}

static expr_value_t *op_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i <= arg1->i);

	return res;
}

static expr_value_t *op_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i > arg1->i);

	return res;
}

static expr_value_t *op_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i >= arg1->i);

	return res;
}

static expr_value_t *op_bit_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, &);

	return res;
}

static expr_value_t *op_bit_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, |);

	return res;
}

static expr_value_t *op_bit_xor(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	INT_OP(arg0, arg1, res, ^);

	return res;
}

static expr_value_t *op_log_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i && arg1->i);

	return res;
}

static expr_value_t *op_log_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->i || arg1->i);

	return res;
}

static array_t *add_array(expr_value_t *arr, expr_value_t *arg){
	expr_value_t *v;


	if(arg->is_array){
		vector_for_each(&arg->array->items, v){
			if(array_add(&arr->array, v) == 0x0)
				break;
		}
	}
	else
		array_add(&arr->array, arg);

	if(arr->array != 0x0 && array_cache_add(&arr->array) == 0x0)
		return 0x0;

	return arr->array;
}

static char const *add_str(char const *s0, char const *s1){
	char r[strlen(s0) + strlen(s1) + 1];


	sprintf(r, "%s%s", s0, s1);

	return str_cache_add(r);
}
