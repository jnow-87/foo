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
#include <attr.h>


/* macros */
#define EXPR_INT_BASE			unsigned long int
#define EXPR_INT_FIXED(size)	ET_INT##size
#define EXPR_ARRAY_UNLIMITED	((size_t)-1)

#define EXPR_LITERAL(_type, _is_array, _value) \
	(&(expr_arg_t){ \
		.type = _type, \
		.is_array = _is_array, \
		.value = _value, \
	})

#define EXPR_INT(size, val)	EXPR_LITERAL( \
	EXPR_INT_FIXED(size), \
	false, \
	(expr_value_t){ .i = val } \
)

#define EXPR_ADDR(val)	EXPR_LITERAL( \
	ET_ADDR, \
	false, \
	(expr_value_t){ .p = val } \
)

#define EXPR_STR(val)	EXPR_LITERAL( \
	ET_STRING, \
	false, \
	(expr_value_t){ .p = val } \
)

#define EXPR_ARRAY(type)	EXPR_LITERAL( \
	type, \
	true, \
	(expr_value_t){ \
		.array.items = VECTOR_INITIALISER(sizeof(expr_value_t)), \
		.array.limit = EXPR_ARRAY_UNLIMITED, \
	} \
)


/* incomplete types */
struct expr_t;
struct expr_arg_t;


/* types */
typedef struct expr_t * (*expr_op_t)(struct expr_t *arg0, struct expr_t *arg1, struct expr_t *res);

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

typedef struct expr_t{
	expr_op_t op;
	attr_vec_t *ctx;

	struct expr_arg_t *arg0,
					  *arg1;
} expr_t;

typedef union{
	EXPR_INT_BASE i;
	void *p;
	expr_t expr;

	struct{
		vector_t items;
		size_t limit;
	} array;
} expr_value_t;


typedef struct expr_arg_t{
	expr_type_t type;
	bool is_array;
	expr_value_t value;
} expr_arg_t;


/* prototypes */
expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1);
expr_t *expr_alloc(expr_t *expr);
void expr_destroy(expr_t *expr);

int expr_array_add(expr_t *array, expr_t *expr);
int expr_copy(expr_t *dest, expr_t *src);

expr_t *expr_evaluate(expr_t *expr, expr_t *result);

expr_t *expr_literal(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_add(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_sub(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_mul(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_div(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_lshift(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_rshift(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_mod(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_eq(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_neq(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_lesser(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_lesser_eq(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_greater(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_greater_eq(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_bit_and(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_bit_or(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_bit_xor(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_log_and(expr_t *arg0, expr_t *arg1, expr_t *res);
expr_t *expr_log_or(expr_t *arg0, expr_t *arg1, expr_t *res);


#endif // DEVTREE_EXPR_H
