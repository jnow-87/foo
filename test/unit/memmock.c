/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <test/test.h>
#include <test/memory.h>


/* local functions */
TEST(memmock){
	int r = 0;
	void *p;


	/* disabled memory mock */
	memmock_alloc_fail = -1;
	r += TEST_PTR_NEQ(p = malloc(16), 0x0); free(p);
	r += TEST_PTR_NEQ(p = calloc(1, 16), 0x0); free(p);

	/* immediate fail */
	memmock_alloc_fail = 0;
	r += TEST_PTR_EQ(malloc(16), 0x0);
	r += TEST_PTR_NEQ(p = malloc(16), 0x0); free(p);

	memmock_alloc_fail = 0;
	r += TEST_PTR_EQ(calloc(1, 16), 0x0);
	r += TEST_PTR_NEQ(p = calloc(1, 16), 0x0); free(p);

	/* fail after some attempts */
	// successful allocations cannot be tested since the
	// test macro itself calls malloc
	memmock_alloc_fail = 3;

	free(malloc(16));
	free(malloc(16));
	free(malloc(16));

	r += TEST_PTR_EQ(malloc(16), 0x0);
	r += TEST_PTR_NEQ(p = malloc(16), 0x0); free(p);

	memmock_alloc_fail = 3;

	free(calloc(1, 16));
	free(calloc(1, 16));
	free(calloc(1, 16));

	r += TEST_PTR_EQ(calloc(1, 16), 0x0);
	r += TEST_PTR_NEQ(p = calloc(1, 16), 0x0); free(p);

	return -r;
}
