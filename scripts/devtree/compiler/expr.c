/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/math.h>
#include <sys/vector.h>
#include <attr.h>
#include <expr.h>
#include <devtree.tab.h>


// TODO
// 	- make expr_create() to evaluate an expression of args are literals
// 	- add attr type "reference"
// 	- update expr_evaluate() to throw an error if a reference cannot be resolved

/* global functions */
expr_t *expr_create(expr_op_t op, expr_t *arg0, expr_t *arg1){
	expr_t *expr;


	// TODO consider who should free arg0 and arg1 if both are literals
	// TODO check if both args are literals, if so, evaluate the expression instead
	if(arg0->op == expr_literal && arg1->op == expr_literal){
		expr = op(arg0, arg1, arg0);
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

int expr_array_add(expr_t *array, expr_t *expr){
	if(!array->arg0->is_array)
		return devtree_parser_error("appending to something not an array");

	if(vector_add(&array->arg0->value.array.items, (expr_value_t){ .expr = expr }) != 0)
		return devtree_parser_error("adding to array failed");

	array->arg0->value.array.limit = MAX(array->value.array.limit, array->value.array.items.size);

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
	else if(src->type == AT_STRING){
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

expr_t *expr_evaluate(expr_t *expr, expr_t *result){
	expr_t arg0,
		   arg1;


	if(expr->op == expr_literal)
		return expr_literal(expr, 0x0, result);

	expr_evaluate(expr, &arg0);
	expr_evaluate(expr, &arg1);

	return expr->op(&arg0, &arg1, result);
}

expr_t *expr_literal(expr_t *arg0, expr_t *arg1, expr_t *res){
	*res = *arg0;

	return res;
}

expr_t *expr_add(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_sub(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_mul(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_div(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_lshift(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_rshift(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}

expr_t *expr_mod(expr_t *arg0, expr_t *arg1, expr_t *res){
	return res;
}
