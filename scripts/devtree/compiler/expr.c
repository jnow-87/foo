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


/* types */
typedef expr_value_t *(*op_cb_t)(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);


/* local/static prototypes */
static expr_value_t *resolve_ref(expr_value_t *arg, void *ctx, expr_value_t *res);

static int types_compatible(expr_type_t t0, expr_type_t t1);
static int op_defined(expr_op_t op, expr_value_t *val);
static int range_check(expr_value_t *value, expr_type_t type);

static size_t type_size(expr_type_t type);
static char const *type_name(expr_type_t type);

static char const *op_name(expr_op_t op);

static expr_value_t *op_literal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_reference(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_subtract(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_multiply(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_divide(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_left_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_right_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_modulo(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_not_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_bit_xor(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_log_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
static expr_value_t *op_log_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);


/* static variables */
static op_cb_t ops[] = {
	[EOP_LITERAL] = op_literal,
	[EOP_REFERENCE] = op_reference,
	[EOP_ADD] = op_add,
	[EOP_SUBTRACT] = op_subtract,
	[EOP_MULTIPLY] = op_multiply,
	[EOP_DIVIDE] = op_divide,
	[EOP_LEFT_SHIFT] = op_left_shift,
	[EOP_RIGHT_SHIFT] = op_right_shift,
	[EOP_MODULO] = op_modulo,
	[EOP_EQUAL] = op_equal,
	[EOP_UNEQUAL] = op_not_equal,
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


//static int type_cast(attr_t *attr, attr_type_t type, attr_flags_t flags);
//static bool type_is_int(attr_type_t type);
//
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
	if(arg0->op == EOP_LITERAL && arg1->op == EOP_LITERAL){
		ops[EOP_LITERAL](arg0->arg0, arg1->arg0, expr->arg0);
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

expr_value_t *expr_evaluate(expr_t *expr, expr_value_t *result, void *ctx){
	expr_value_t arg0,
				 arg1;


	if(expr->op == EOP_LITERAL)
		return op_literal(expr->arg0, 0x0, result);

	if(expr->op == EOP_REFERENCE)
		return resolve_ref(expr->arg0, ctx, result);

	if(expr_evaluate(expr, &arg0, ctx) == 0x0 || expr_evaluate(expr, &arg1, ctx) == 0x0)
		return 0x0;

	if(types_compatible(arg0.type, arg1.type) != 0)
		return 0x0;

	if(op_defined(expr->op, &arg0) != 0 || op_defined(expr->op, &arg1) != 0)
		return 0x0;

	return ops[expr->op](&arg0, &arg1, result);
}


/* local functions */
static expr_value_t *resolve_ref(expr_value_t *arg, void *ctx, expr_value_t *res){
	attr_t *ref;


	ref = attr_query(ctx, arg->p, false);

	if(ref == 0x0)
		return 0x0;

	*res = *ref->value->arg0;

	return res;
}

static int types_compatible(expr_type_t t0, expr_type_t t1){
	bool ints = EXPR_TYPE_IS_INT(t0) || EXPR_TYPE_IS_INT(t1),
		 addrs = t0 == ET_ADDR || t1 == ET_ADDR,
		 strs = t0 == ET_STRING || t1 == ET_STRING,
		 undefs = t0 == ET_UNDEF || t1 == ET_UNDEF,
		 exprs = t0 == ET_EXPR || t1 == ET_EXPR;


	if((t0 == t1 || ((ints || addrs) && !strs)) && !exprs && !undefs)
		return 0;

	return devtree_parser_error("incompatible types %s and %s", type_name(t0), type_name(t1));
}

static int op_defined(expr_op_t op, expr_value_t *val){
	static unsigned int valid_ops[] = {
		[ET_UNDEF] = 0x0,
		[ET_INT8] = EOP_ALL,
		[ET_INT16] = EOP_ALL,
		[ET_INT32] = EOP_ALL,
		[ET_INT64] = EOP_ALL,
		[ET_ADDR] = EOP_LITERAL | EOP_REFERENCE | EOP_ADD | EOP_SUBTRACT | EOP_EQUAL | EOP_UNEQUAL | EOP_LESSER | EOP_LESSER_EQUAL | EOP_GREATER | EOP_GREATER_EQUAL,
		[ET_STRING] = EOP_LITERAL | EOP_REFERENCE | EOP_ADD,
		[ET_EXPR] = 0x0,
		[ET_NUM_TYPES] = 0x0,
	};


	if(((valid_ops[val->type] & op) == 0) || (val->is_array && op != EOP_ADD)){
		return devtree_parser_error("undefined operation %s for type %s%s"
			, op_name(op)
			, type_name(val->type)
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

	lim = (((EXPR_INT_T)1 << ((type_size(type) * 8) - 1)) << 1) - 1;

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

static size_t type_size(expr_type_t type){
	switch(type){
	case ET_ADDR:	return sizeof(void*);
	case ET_INT8:	return 1;
	case ET_INT16:	return 2;
	case ET_INT32:	return 4;
	case ET_INT64:	return 8;
	default:		return 1;
	}
}

static char const *type_name(expr_type_t type){
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

static char const *op_name(expr_op_t op){
	switch(op){
	case EOP_LITERAL:		return "literal";
	case EOP_REFERENCE:		return "reference";
	case EOP_ADD:			return "add";
	case EOP_SUBTRACT:		return "subtract";
	case EOP_MULTIPLY:		return "multiply";
	case EOP_DIVIDE:		return "divide";
	case EOP_LEFT_SHIFT:	return "left_shift";
	case EOP_RIGHT_SHIFT:	return "right_shift";
	case EOP_MODULO:		return "modulo";
	case EOP_EQUAL:			return "equal";
	case EOP_UNEQUAL:		return "unequal";
	case EOP_LESSER:		return "lesser";
	case EOP_LESSER_EQUAL:	return "lesser equal";
	case EOP_GREATER:		return "greater";
	case EOP_GREATER_EQUAL:	return "greater equal";
	case EOP_BIT_AND:		return "bit_and";
	case EOP_BIT_OR:		return "bit_or";
	case EOP_BIT_XOR:		return "bit_xor";
	case EOP_LOG_AND:		return "log_and";
	case EOP_LOG_OR:		return "log_or";
	default:				return "unknown";
	}
}

static expr_value_t *op_literal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	*res = *arg0;

	return res;
}

static expr_value_t *op_reference(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	// NOTE this function is only used as "enum" for expr_op_t, the actual implementation
	// 		is in resolve_ref(), since it needs a ctx to resolve the reference, which the
	// 		other operations don't need and thus is not part of the interface
	return 0x0;
}

static expr_value_t *op_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_subtract(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_multiply(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_divide(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_left_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_right_shift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_modulo(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_not_equal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_bit_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_bit_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_bit_xor(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_log_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
	return res;
}

static expr_value_t *op_log_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res){
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
//
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
