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
#include "array.h"


/* macros */
#define EXPR_INT_T				unsigned long int
#define EXPR_INT_FIXED(size)	ET_INT##size

#define EXPR_TYPE_ENUM(v, is_int)	(v << 1 | (is_int ? 0x1 : 0x0))
#define EXPR_TYPE_IS_INT(type)		((bool)(type & 0x1))

#define EXPR_VALUE(_type, _is_array, _field, _value) ((expr_value_t){ \
	.type = _type, \
	.is_array = _is_array, \
	._field = _value, \
})

#define EXPR(_op, _literal, _arg0, _arg1) ((expr_t){ \
	.op = _op, \
	.literal = _literal, \
	.arg0 = _arg0, \
	.arg1 = _arg1, \
})

#define EXPR_CALC(op, arg0, arg1)	EXPR(op, { 0 }, arg0, arg1)
#define EXPR_REF(_name)				EXPR(EOP_REFERENCE, EXPR_VALUE(ET_STRING, false, p, _name), 0x0, 0x0)
#define EXPR_LITERAL(val) 			EXPR(EOP_LITERAL, val, 0x0, 0x0)

#define EXPR_INT(size, val)		EXPR_LITERAL(EXPR_VALUE(EXPR_INT_FIXED(size), false, i, val))
#define EXPR_ADDR(val)			EXPR_LITERAL(EXPR_VALUE(ET_ADDR, false, p, val))
#define EXPR_STR(val)			EXPR_LITERAL(EXPR_VALUE(ET_STRING, false, p, val))
#define EXPR_ARRAY(type, val)	EXPR_LITERAL(EXPR_VALUE(type, true, array, val))


/* incomplete types */
struct node_attrvec_t;


/* types */
typedef enum expr_type_t{
	ET_UNDEF = EXPR_TYPE_ENUM(0x0, false),
	ET_INT8 = EXPR_TYPE_ENUM(0x1, true),
	ET_INT16 = EXPR_TYPE_ENUM(0x2, true),
	ET_INT32 = EXPR_TYPE_ENUM(0x4, true),
	ET_INT64 = EXPR_TYPE_ENUM(0x8, true),
	ET_ADDR = EXPR_TYPE_ENUM(0x10, false),
	ET_STRING = EXPR_TYPE_ENUM(0x20, false),
} expr_type_t;

typedef enum{
	EOP_UNDEF = 0x0,
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
	expr_type_t type;
	bool is_array;

	union{
		EXPR_INT_T i;
		void const *p;
		array_t *array;
	};
} expr_value_t;

typedef struct expr_t{
	expr_op_t op;

	expr_value_t literal;
	struct expr_t *arg0,
				  *arg1;
} expr_t;


/* prototypes */
expr_t *expr_alloc(expr_t *expr);
void expr_reset(expr_t *expr);
void expr_free(expr_t *expr);

expr_value_t *expr_value_mv(expr_value_t *dest, expr_value_t *src);
void expr_value_reset(expr_value_t *value);

int expr_evaluate(expr_t *expr, expr_value_t *result, struct node_attrvec_t *ctx);

expr_type_t expr_type(expr_t *expr);
int expr_type_check(expr_t *expr, expr_type_t type, bool is_array, size_t array_limit);
size_t expr_type_size(expr_type_t type);
char const *expr_type_name(expr_type_t type);

int expr_array_add(expr_t *array, expr_t *expr);


#endif // DEVTREE_EXPR_H
