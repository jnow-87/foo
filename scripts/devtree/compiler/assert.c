/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <parser.tab.h>
#include "assert.h"
#include "expr.h"
#include "node.h"


/* global functions */
assert_t *assert_alloc(expr_t *expr, char const *msg){
	assert_t *a;


	a = malloc(sizeof(assert_t));

	if(a == 0x0)
		goto err;

	a->expr = expr;
	a->msg = msg;

	return a;


err:
	devtree_parser_error("assert allocation failed");

	return 0x0;
}

void asserts_free(assert_t *asserts){
	assert_t *a;


	list_for_each(asserts, a){
		expr_free(a->expr);
		free(a);
	}
}

int asserts_eval(assert_t *asserts, node_attrvec_t *ctx, char const *obj_name){
	assert_t *assert;
	expr_value_t r;

	list_for_each(asserts, assert){
		if(expr_evaluate(assert->expr, &r, ctx) == 0x0 || !EXPR_TYPE_IS_INT(r.type) || r.is_array || r.i == 0){
			// TODO check if any of the expr_value_free() calls in the project are needed
			// 		they were introduced to free string value, but since strings are now
			// 		handled via strmap this is no longer needed, can any of the other
			// 		cases handled in expr_value_free() occur
			expr_value_reset(&r);

			return devtree_parser_error("%s: assertion failed: %s", obj_name, assert->msg);
		}
	}

	return 0;
}
