/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_EXPR_H
#define DEVTREE_EXPR_H


#include <stdbool.h>
#include <sys/vector.h>


/* macros */
#define EXPR_INT_T				unsigned long int
#define EXPR_INT_FIXED(size)	ET_INT##size
#define EXPR_ARRAY_UNLIMITED	((size_t)-1)

#define EXPR_ARRAY_INITIALISER() ((expr_array_t){ \
	.items = VECTOR_INITIALISER(sizeof(expr_value_t)), \
	.limit = EXPR_ARRAY_UNLIMITED, \
})

#define EXPR_ARG(_type, _is_array, _field, _value) (&(expr_value_t){ \
	.type = _type, \
	.is_array = _is_array, \
	._field = _value, \
})

#define EXPR_LITERAL(_arg) (&(expr_t){ \
	.op = expr_literal, \
	.arg0 = _arg, \
	.arg1 = 0x0, \
})

#define EXPR_INT(size, val)	EXPR_LITERAL(EXPR_ARG(EXPR_INT_FIXED(size), false, i, val))
#define EXPR_ADDR(val)		EXPR_LITERAL(EXPR_ARG(ET_ADDR, false, p, val))
#define EXPR_STR(val)		EXPR_LITERAL(EXPR_ARG(ET_STRING, false, p, val))
#define EXPR_ARRAY(type)	EXPR_LITERAL(EXPR_ARG(type, true, array, EXPR_ARRAY_INITIALISER()))


/* incomplete types */
struct expr_value_t;


/* types */
typedef struct expr_value_t * (*expr_op_t)(struct expr_value_t *arg0, struct expr_value_t *arg1, struct expr_value_t *res);

typedef enum{
	ET_UNDEF = 0,
	ET_INT8,
	ET_INT16,
	ET_INT32,
	ET_INT64,
	ET_ADDR,
	ET_STRING,
	ET_EXPR,
} expr_type_t;

typedef struct{
	vector_t items;
	size_t limit;
} expr_array_t;

typedef struct{
	expr_op_t op;

	struct expr_value_t *arg0,
						*arg1;
} expr_t;

typedef struct expr_value_t{
	expr_type_t type;
	bool is_array;

	union{
		EXPR_INT_T i;
		void *p;
		expr_t expr;
		expr_array_t array;
	};
} expr_value_t;


/* prototypes */
expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1);
expr_t *expr_alloc(expr_t *expr);
void expr_destroy(expr_t *expr);

size_t expr_type_size(expr_type_t type);
int expr_type_check(expr_t *expr, expr_type_t type);
char const *expr_type_name(expr_type_t type);

int expr_array_add(expr_t *array, expr_t *expr);
int expr_copy(expr_t *dest, expr_t *src);

expr_value_t *expr_evaluate(expr_t *expr, expr_value_t *result, void *ctx);	// TODO make ctx a attr_vec_t

expr_value_t *expr_literal(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_reference(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_add(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_sub(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_mul(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_div(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_lshift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_rshift(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_mod(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_neq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_lesser(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_lesser_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_greater(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_greater_eq(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_bit_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_bit_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_bit_xor(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_log_and(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);
expr_value_t *expr_log_or(expr_value_t *arg0, expr_value_t *arg1, expr_value_t *res);


#endif // DEVTREE_EXPR_H
