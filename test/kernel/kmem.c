/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <test/test.h>


/* local functions */
TEST(iskheap){
	unsigned int n;
	void *p;


	n = 0;

	n += TEST_INT_EQ(iskheap(0x0), false);
	n += TEST_INT_EQ(iskheap(&n), false);

	p = kmalloc(2);
	n += TEST_PTR_NEQ(p, 0x0);
	n += TEST_INT_EQ(iskheap(p), true);
	kfree(p);

	return -n;
}
