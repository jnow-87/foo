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
	int n;
	vector_t v;


	n = 0;

	n += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);
	n += TEST_INT_EQ(v.capacity, 20);
	n += TEST_INT_EQ(v.size, 0);
	n += TEST_INT_EQ(v.dt_size, sizeof(int));

	vector_destroy(&v);

	test_memory_init();

	test_malloc_fail_at = 1;
	n += TEST_INT_EQ(vector_init(&v, 0, 0), -1);

	return -n;
}

TEST(vector_destroy){
	int n;
	vector_t v;


	n = 0;
	n += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);
	vector_destroy(&v);

	n += TEST_INT_EQ(v.capacity, 0);
	n += TEST_INT_EQ(v.size, 0);
	n += TEST_INT_EQ(v.dt_size, sizeof(int));

	return -n;
}

TEST(vector_add){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 2);

	n = 0;
	n += TEST_INT_EQ(r, 0);

	r = 10;
	r = vector_add(&v, &r);
	n += TEST_INT_EQ(r, 0);
	n += TEST_INT_EQ(v.capacity, 2);
	n += TEST_INT_EQ(v.size, 1);

	r = 20;
	r = vector_add(&v, &r);
	n += TEST_INT_EQ(r, 0);
	n += TEST_INT_EQ(v.capacity, 2);
	n += TEST_INT_EQ(v.size, 2);

	r = 30;
	r = vector_add(&v, &r);
	n += TEST_INT_EQ(r, 0);
	n += TEST_INT_EQ(v.capacity, 4);
	n += TEST_INT_EQ(v.size, 3);

	n += TEST_INT_EQ(((int*)v.data)[0], 10);
	n += TEST_INT_EQ(((int*)v.data)[1], 20);
	n += TEST_INT_EQ(((int*)v.data)[2], 30);

	test_memory_init();

	test_malloc_fail_at = 1;
	n += TEST_INT_EQ(vector_add(&v, &r), 0);
	n += TEST_INT_EQ(vector_add(&v, &r), -1);

	test_memory_reset();

	vector_destroy(&v);

	return -n;
}

TEST(vector_rm){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 20);

	n = 0;
	n += TEST_INT_EQ(r, 0);

	r = 10; r = vector_add(&v, &r);
	r = 20; r = vector_add(&v, &r);
	r = 30; r = vector_add(&v, &r);
	r = 40; r = vector_add(&v, &r);
	r = 50; r = vector_add(&v, &r);

	vector_rm(&v, 0);
	n += TEST_INT_EQ(v.capacity, 20);
	n += TEST_INT_EQ(v.size, 4);
	n += TEST_INT_EQ(((int*)v.data)[0], 20);
	n += TEST_INT_EQ(((int*)v.data)[1], 30);
	n += TEST_INT_EQ(((int*)v.data)[2], 40);
	n += TEST_INT_EQ(((int*)v.data)[3], 50);

	vector_rm(&v, 3);
	n += TEST_INT_EQ(v.capacity, 20);
	n += TEST_INT_EQ(v.size, 3);
	n += TEST_INT_EQ(((int*)v.data)[0], 20);
	n += TEST_INT_EQ(((int*)v.data)[1], 30);
	n += TEST_INT_EQ(((int*)v.data)[2], 40);

	vector_rm(&v, 1);
	n += TEST_INT_EQ(v.capacity, 20);
	n += TEST_INT_EQ(v.size, 2);
	n += TEST_INT_EQ(((int*)v.data)[0], 20);
	n += TEST_INT_EQ(((int*)v.data)[1], 40);

	vector_rm(&v, 3);

	vector_destroy(&v);

	return -n;
}

TEST(vector_get){
	int n;
	int r;
	vector_t v;


	n = 0;

	n += TEST_INT_EQ(vector_init(&v, sizeof(int), 20), 0);

	r = 10; r = vector_add(&v, &r);
	r = 20; r = vector_add(&v, &r);
	r = 30; r = vector_add(&v, &r);
	r = 40; r = vector_add(&v, &r);
	r = 50; r = vector_add(&v, &r);

	n += TEST_INT_EQ(*((int*)vector_get(&v, 0)), 10);
	n += TEST_INT_EQ(*((int*)vector_get(&v, 2)), 30);
	n += TEST_INT_EQ(*((int*)vector_get(&v, 4)), 50);

	n += TEST_PTR_EQ(vector_get(&v, 5), 0x0);

	vector_destroy(&v);

	return -n;
}

TEST(vector_foreach){
	int n;
	int r;
	int *p;
	vector_t v;


	r = vector_init(&v, sizeof(int), 20);

	n = 0;
	n += TEST_INT_EQ(r, 0);

	r = 10; r = vector_add(&v, &r); n += TEST_INT_EQ(r, 0);
	r = 20; r = vector_add(&v, &r); n += TEST_INT_EQ(r, 0);
	r = 30; r = vector_add(&v, &r); n += TEST_INT_EQ(r, 0);
	r = 40; r = vector_add(&v, &r); n += TEST_INT_EQ(r, 0);
	r = 50; r = vector_add(&v, &r); n += TEST_INT_EQ(r, 0);

	r = 10;

	vector_for_each(&v, p){
		n += TEST_INT_EQ(r, *p);
		r += 10;
	}

	vector_destroy(&v);

	return -n;
}
