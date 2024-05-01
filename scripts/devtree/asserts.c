/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <parser.tab.h>
#include <assert.h>


/* global functions */
assert_t *assert_alloc(char const *expr, char const *msg){
	assert_t *assert;


	assert = malloc(sizeof(assert_t));

	if(assert == 0x0)
		goto err;

	assert->expr = expr;
	assert->msg = msg;

	return assert;


err:
	devtree_parser_error("assert allocation failed");

	return 0x0;
}
