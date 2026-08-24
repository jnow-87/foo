/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <parser.tab.h>
#include "assert.h"


/* global functions */
assert_t *assert_create(expr_t *expr, char const *msg){
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
