/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "attr.h"
#include "attrvec.h"
#include "expr.h"


/* macros */
#define INT_OP(a0, a1, r, op){ \
	r->i = a0->i op a1->i; \
	\
	if(range_check(r, r->type) != 0) \
		return 0x0; \
}


/* types */
typedef expr_value_t *(*op_cb_t)(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);


/* local/static prototypes */
static expr_value_t *resolve_ref(expr_value_t *arg, attrvec_t *ctx, expr_value_t *res);

static int types_compatible(expr_type_t t0, expr_type_t t1, bool any_array);
static int op_defined(expr_op_t op, expr_value_t *val);
static int range_check(expr_value_t *value, expr_type_t type);
static int copy_value(expr_value_t *dest, expr_value_t *src);
static void free_value(expr_value_t *val);

static expr_value_t *op_literal(expr_value_t *arg0, expr_value_t *res);
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

static int add_array(expr_value_t *arr, expr_value_t *arg);


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

void expr_free(expr_t *expr){
	if(expr == 0x0)
		return;

	free_value(&expr->arg0);
	free_value(&expr->arg1);

	free(expr);
}

expr_value_t *expr_evaluate(expr_t *expr, expr_value_t *result, attrvec_t *ctx){
	expr_value_t arg0,
				 arg1;


	if(expr->op == EOP_LITERAL)
		return op_literal(&expr->arg0, result);

	if(expr->op == EOP_REFERENCE)
		return resolve_ref(&expr->arg0, ctx, result);

	if(expr_evaluate(expr->arg0.expr, &arg0, ctx) == 0x0 || expr_evaluate(expr->arg1.expr, &arg1, ctx) == 0x0)
		return 0x0;

	if(types_compatible(arg0.type, arg1.type, arg0.is_array || arg1.is_array) != 0)
		return 0x0;

	if(op_defined(expr->op, &arg0) != 0 || op_defined(expr->op, &arg1) != 0)
		return 0x0;

	result->type = expr_type(expr);
	result->is_array = arg0.is_array || arg1.is_array;

	if(ops[expr->op](&arg0, &arg1, result) == 0x0){
		devtree_parser_error("operation %s failed", op_name[expr->op]);

		return 0x0;
	}

	return result;
}

// TODO maybe update to already return the result
// 		then expr_init() might no longer be needed, since its call to expr_evaluate() is no longer needed
int expr_evaluable(expr_op_t op, expr_value_t *arg0, expr_value_t *arg1, attrvec_t *ctx){
	expr_t e = EXPR(op, *arg0, *arg1);
	expr_value_t r;


	return -(expr_evaluate(&e, &r, ctx) == 0x0);
}

expr_type_t expr_type(expr_t *expr){
	expr_type_t t0,
				t1;


	if(expr->op & (EOP_LITERAL | EOP_REFERENCE))
		return expr->arg0.type;

	if(expr->op & (EOP_EQUAL | EOP_UNEQUAL | EOP_LESSER | EOP_LESSER_EQUAL | EOP_GREATER | EOP_GREATER_EQUAL | EOP_LOG_AND | EOP_LOG_OR))
		return ET_INT8;

	t0 = expr_type(expr->arg0.expr);
	t1 = expr_type(expr->arg1.expr);

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

int expr_type_check(expr_t *expr, expr_type_t type){
	// TODO is a range check necessary?
//	if(range_check(attr, &v) != 0)
//		return -1;

	return types_compatible(expr_type(expr), type, false);
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
	case ET_EXPR:	return "expr";
	default:		return "undef";
	}
}

int expr_array_add(expr_t *array, expr_t *expr){
	expr_array_t *arr = &array->arg0.array;


	if(!array->arg0.is_array)
		return devtree_parser_error("appending to something not an array");

	if(array->arg0.type == ET_UNDEF)
		array->arg0.type = expr->arg0.type;

	if(types_compatible(array->arg0.type, expr->arg0.type, true) != 0)
		return -1;

	if(vector_add(&arr->items, &(expr_value_t){ .expr = expr }) != 0)
		return devtree_parser_error("adding to array failed");

	arr->limit = MAX(arr->limit, arr->items.size);

	return 0;
}

int expr_copy(expr_t *dest, expr_t *src){
	dest->op = src->op;

	if(copy_value(&dest->arg0, &src->arg0) != 0)
		goto err_0;

	if(copy_value(&dest->arg1, &src->arg1) != 0)
		goto err_1;

	return 0;


err_1:
	free_value(&dest->arg0);

err_0:
	return -1;
}

#include <stdio.h>
void expr_print(expr_t *expr, int indent){
	printf("\n%*.*s%s: ", indent, indent, "", op_name[expr->op]);
	expr_value_print(&expr->arg0, indent);

	if((expr->op & (EOP_LITERAL | EOP_REFERENCE)) == 0)
		expr_value_print(&expr->arg1, indent);
}

void expr_value_print(expr_value_t *value, int indent){
	if(value->is_array){
		printf("array");
		return;
	}

	switch(value->type){
	case ET_UNDEF:	printf("undef"); break;
	case ET_ADDR:	printf("%p", value->p); break;
	case ET_STRING:	printf("%s", value->p); break;
	case ET_EXPR:	expr_print(value->expr, indent + 1); break;
	default:		printf("%d", value->i); break;
	}
}


/* local functions */
static expr_value_t *resolve_ref(expr_value_t *arg, attrvec_t *ctx, expr_value_t *res){
	attr_t *ref;


	if(ctx == 0x0)
		return 0x0;

	ref = attrvec_query(ctx, arg->p, false);

	if(ref == 0x0)
		return 0x0;

	*res = ref->value->arg0;

	return res;
}

static int types_compatible(expr_type_t t0, expr_type_t t1, bool any_array){
	uint8_t ints = EXPR_TYPE_IS_INT(t0) + EXPR_TYPE_IS_INT(t1),
			addrs = (t0 == ET_ADDR) + (t1 == ET_ADDR),
			exprs = (t0 == ET_EXPR) + (t1 == ET_EXPR),
			undefs = (t0 == ET_UNDEF) + (t1 == ET_UNDEF);


	if((exprs + undefs == 0) && ((t0 == t1) || (!any_array && (ints + addrs == 2)) || (ints == 2)))
		return 0;

	return devtree_parser_error("incompatible types %s and %s", expr_type_name(t0), expr_type_name(t1));
}

static int op_defined(expr_op_t op, expr_value_t *val){
	static unsigned int valid_ops[] = {
		[ET_UNDEF] = 0x0,
		[ET_INT8] = EOP_ALL,
		[ET_INT16] = EOP_ALL,
		[ET_INT32] = EOP_ALL,
		[ET_INT64] = EOP_ALL,
		[ET_ADDR] = EOP_LITERAL | EOP_SUBTRACT | EOP_REFERENCE | EOP_ADD | EOP_EQUAL | EOP_UNEQUAL | EOP_LESSER | EOP_LESSER_EQUAL | EOP_GREATER | EOP_GREATER_EQUAL,
		[ET_STRING] = EOP_LITERAL | EOP_REFERENCE | EOP_ADD,
		[ET_EXPR] = 0x0,
		[ET_NUM_TYPES] = 0x0,
	};


	if(((valid_ops[val->type] & op) == 0) || (val->is_array && op != EOP_ADD)){
		return devtree_parser_error("undefined operation %s for type %s%s"
			, op_name[op]
			, expr_type_name(val->type)
			, val->is_array ? "array" : ""
		);
	}

	return 0;
}

static int range_check(expr_value_t *value, expr_type_t type){
	EXPR_INT_T lim;
	vector_t *arr;


	if(!EXPR_TYPE_IS_INT(type))
		return 0;

	lim = (((EXPR_INT_T)1 << ((expr_type_size(type) * 8) - 1)) << 1) - 1;

	if(value->is_array){
		arr = &value->array.items;

		vector_for_each(arr, value){
			if(value->i > lim)
				goto err;
		}
	}
	else if(value->i > lim)
		goto err;

	return 0;


err:
	return devtree_parser_error("integer out of range %lu > %lu", value->i, lim);
}

static int copy_value(expr_value_t *dest, expr_value_t *src){
	*dest = *src;

	if(src->is_array){
		dest->array.limit = src->array.limit;

		if(vector_copy(&dest->array.items, &src->array.items) != 0)
			goto err;

		return 0;
	}

	switch(src->type){
	case ET_STRING:
		dest->p = strdup(src->p);

		if(dest->p == 0x0)
			goto err;

		break;

	case ET_EXPR:
		dest->expr = expr_alloc(0x0);

		if(dest->expr == 0x0 || expr_copy(dest->expr, src->expr) != 0)
			goto err;

	default:
		break;
	}

	return 0;


err:
	return devtree_parser_error("copy expression");
}

static void free_value(expr_value_t *val){
	expr_value_t *el;


	if(val->is_array){
		vector_for_each(&val->array.items, el)
			expr_free(el->expr);

		vector_destroy(&val->array.items);

		return;
	}

	switch(val->type){
	case ET_STRING:
		free(val->p);
		break;

	case ET_EXPR:
		expr_free(val->expr);
		break;

	default:
		break;
	}
}

static expr_value_t *op_literal(expr_value_t *arg, expr_value_t *res){
	*res = *arg;

	return res;
}

static expr_value_t *op_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	if(arg0->is_array || arg1->is_array){
		res->is_array = true;
		res->array = EXPR_ARRAY_INITIALISER();

		if(add_array(res, arg0) != 0 || add_array(res, arg1) != 0)
			return 0x0;
	}
	else if(EXPR_TYPE_IS_INT(arg0->type)){
		if(EXPR_TYPE_IS_INT(arg1->type))	INT_OP(arg0, arg1, res, +)
		else if(arg1->type == ET_ADDR)		res->p = arg0->i + arg1->p;
		else								return 0x0;
	}
	else if(arg0->type == ET_STRING){
		res->p = malloc(strlen(arg0->p) + strlen(arg1->p) + 1);

		if(res->p == 0x0){
			devtree_parser_error("out of memory");
			return 0x0;
		}

		sprintf(res->p, "%s%s", arg0->p, arg1->p);
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
	res->i = (arg0->p == arg1->p);

	return res;
}

static expr_value_t *op_unequal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->p != arg1->p);

	return res;
}

static expr_value_t *op_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->p < arg1->p);

	return res;
}

static expr_value_t *op_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->p <= arg1->p);

	return res;
}

static expr_value_t *op_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->p > arg1->p);

	return res;
}

static expr_value_t *op_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	res->i = (arg0->p >= arg1->p);

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

static int add_array(expr_value_t *arr, expr_value_t *arg){
	expr_value_t *v;


	if(!arg->is_array){
		if(vector_add(&arr->array.items, arg) != 0)
			return -1;

		arr->array.limit = MAX(arr->array.limit, arr->array.items.size);

		return 0;
	}

	vector_for_each(&arg->array.items, v){
		if(add_array(arr, v) != 0)
			return -1;
	}

	return 0;
}
