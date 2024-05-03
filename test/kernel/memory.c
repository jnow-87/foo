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
	int r = 0;
	void *p;


	r |= TEST_INT_EQ(iskheap(0x0), false);
	r |= TEST_INT_EQ(iskheap(&r), false);

	p = kmalloc(2);
	r |= TEST_PTR_NEQ(p, 0x0);
	r |= TEST_INT_EQ(iskheap(p), true);
	kfree(p);

	return -r;
}
