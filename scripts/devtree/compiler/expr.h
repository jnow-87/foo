/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_EXPR_H
#define DEVTREE_EXPR_H


#include <sys/types.h>
#include <sys/vector.h>
#include "attrvec.h"


/* macros */
#define EXPR_INT_T				unsigned long int
#define EXPR_INT_FIXED(size)	ET_INT##size
#define EXPR_ARRAY_UNLIMITED	((size_t)-1)

#define EXPR_TYPE_ENUM(v, is_int)	(v << 1 | (is_int ? 0x1 : 0x0))
#define EXPR_TYPE_IS_INT(type)		((bool)(type | 0x1))

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
	.op = EOP_LITERAL, \
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
typedef enum expr_type_t{
	ET_UNDEF = EXPR_TYPE_ENUM(0, false),
	ET_INT8 = EXPR_TYPE_ENUM(1, true),
	ET_INT16 = EXPR_TYPE_ENUM(2, true),
	ET_INT32 = EXPR_TYPE_ENUM(3, true),
	ET_INT64 = EXPR_TYPE_ENUM(4, true),
	ET_ADDR = EXPR_TYPE_ENUM(5, false),
	ET_STRING = EXPR_TYPE_ENUM(6, false),
	ET_EXPR = EXPR_TYPE_ENUM(7, false),
	ET_NUM_TYPES
} expr_type_t;

typedef enum{
	EOP_LITERAL = 0x1,
	EOP_REFERENCE = 0x2,
	EOP_ADD = 0x4,
	EOP_SUBTRACT = 0x8,
	EOP_MULTIPLY = 0x10,
	EOP_DIVIDE = 0x20,
	EOP_LEFT_SHIFT = 0x40,
	EOP_RIGHT_SHIFT = 0x80,
	EOP_MODULO = 0x100,
	EOP_EQUAL = 0x200,
	EOP_UNEQUAL = 0x400,
	EOP_LESSER = 0x800,
	EOP_LESSER_EQUAL = 0x1000,
	EOP_GREATER = 0x2000,
	EOP_GREATER_EQUAL = 0x4000,
	EOP_BIT_AND = 0x8000,
	EOP_BIT_OR = 0x10000,
	EOP_BIT_XOR = 0x20000,
	EOP_LOG_AND = 0x40000,
	EOP_LOG_OR = 0x80000,
	EOP_ALL = 0xfffff
} expr_op_t;

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

expr_type_t expr_type(expr_t *expr);
int expr_type_check(expr_t *expr, expr_type_t type);
size_t expr_type_size(expr_type_t type);
char const *expr_type_name(expr_type_t type);

int expr_array_add(expr_t *array, expr_t *expr);
int expr_copy(expr_t *dest, expr_t *src);

expr_value_t *expr_evaluate(expr_t *expr, expr_value_t *result, attrvec_t *ctx);


#endif // DEVTREE_EXPR_H
