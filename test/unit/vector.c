/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/vector.h>
#include <sys/types.h>
#include <test/test.h>
#include <test/memory.h>


/* macros */
#define INIT_EL() \
	el0.prev = el0.next = (void*)0x1; \
	el1.prev = el1.next = (void*)0x1; \
	el2.prev = el2.next = (void*)0x1; \
	el3.prev = el3.next = (void*)0x1; \


/* local functions */
TEST(vector_init){
	int r = 0;
	vector_t v;


	r += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);
	r += TEST_INT_EQ(v.capacity, 20);
	r += TEST_INT_EQ(v.size, 0);
	r += TEST_INT_EQ(v.dt_size, sizeof(int));

	vector_destroy(&v);

	memmock_alloc_fail = 0;
	r += TEST_INT_EQ(vector_init(&v, 0, 0), -1);

	return -r;
}

TEST(vector_destroy){
	int r = 0;
	vector_t v;


	r += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);
	vector_destroy(&v);

	r += TEST_INT_EQ(v.capacity, 0);
	r += TEST_INT_EQ(v.size, 0);
	r += TEST_INT_EQ(v.dt_size, sizeof(int));

	return -r;
}

TEST(vector_add){
	int r = 0;
	int x;
	vector_t v;


	r += vector_init(&v, sizeof(int), 2);

	x = 10;
	r += TEST_INT_EQ(vector_add(&v, &x), 0);
	r += TEST_INT_EQ(v.capacity, 2);
	r += TEST_INT_EQ(v.size, 1);

	x = 20;
	r += TEST_INT_EQ(vector_add(&v, &x), 0);
	r += TEST_INT_EQ(v.capacity, 2);
	r += TEST_INT_EQ(v.size, 2);

	x = 30;
	r += TEST_INT_EQ(vector_add(&v, &x), 0);
	r += TEST_INT_EQ(v.capacity, 4);
	r += TEST_INT_EQ(v.size, 3);

	r += TEST_INT_EQ(((int*)v.buf)[0], 10);
	r += TEST_INT_EQ(((int*)v.buf)[1], 20);
	r += TEST_INT_EQ(((int*)v.buf)[2], 30);

	r += TEST_INT_EQ(vector_add(&v, &x), 0);

	memmock_alloc_fail = 0;
	r += TEST_INT_EQ(vector_add(&v, &x), -1);

	vector_destroy(&v);

	return -r;
}

TEST(vector_rm){
	int r = 0;
	int x;
	vector_t v;


	r += vector_init(&v, sizeof(int), 20);

	x = 10; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 20; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 30; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 40; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 50; r += TEST_INT_EQ(vector_add(&v, &x), 0);

	vector_rm(&v, 0);
	r += TEST_INT_EQ(v.capacity, 20);
	r += TEST_INT_EQ(v.size, 4);
	r += TEST_INT_EQ(((int*)v.buf)[0], 20);
	r += TEST_INT_EQ(((int*)v.buf)[1], 30);
	r += TEST_INT_EQ(((int*)v.buf)[2], 40);
	r += TEST_INT_EQ(((int*)v.buf)[3], 50);

	vector_rm(&v, 3);
	r += TEST_INT_EQ(v.capacity, 20);
	r += TEST_INT_EQ(v.size, 3);
	r += TEST_INT_EQ(((int*)v.buf)[0], 20);
	r += TEST_INT_EQ(((int*)v.buf)[1], 30);
	r += TEST_INT_EQ(((int*)v.buf)[2], 40);

	vector_rm(&v, 1);
	r += TEST_INT_EQ(v.capacity, 20);
	r += TEST_INT_EQ(v.size, 2);
	r += TEST_INT_EQ(((int*)v.buf)[0], 20);
	r += TEST_INT_EQ(((int*)v.buf)[1], 40);

	vector_rm(&v, 3);

	vector_destroy(&v);

	return -r;
}

TEST(vector_get){
	int r = 0;
	int x;
	vector_t v;


	r += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);

	x = 10; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 20; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 30; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 40; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 50; r += TEST_INT_EQ(vector_add(&v, &x), 0);

	r += TEST_INT_EQ(*((int*)vector_get(&v, 0)), 10);
	r += TEST_INT_EQ(*((int*)vector_get(&v, 2)), 30);
	r += TEST_INT_EQ(*((int*)vector_get(&v, 4)), 50);

	r += TEST_PTR_EQ(vector_get(&v, 5), 0x0);

	vector_destroy(&v);

	return -r;
}

TEST(vector_foreach){
	int r = 0;
	int x;
	int *p;
	vector_t v;


	r += vector_init(&v, sizeof(int), 20);

	x = 10; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 20; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 30; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 40; r += TEST_INT_EQ(vector_add(&v, &x), 0);
	x = 50; r += TEST_INT_EQ(vector_add(&v, &x), 0);

	x = 10;

	vector_for_each(&v, p){
		r += TEST_INT_EQ(x, *p);
		x += 10;
	}

	vector_destroy(&v);

	return -r;
}
