/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/vector.h>
#include <sys/types.h>
#include <testing/testcase.h>


/* macros */
#define INIT_EL() \
	el0.prev = el0.next = (void*)0x1; \
	el1.prev = el1.next = (void*)0x1; \
	el2.prev = el2.next = (void*)0x1; \
	el3.prev = el3.next = (void*)0x1; \


/* local functions */
static int tc_vector_init(int log){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 20);

	n = 0;
	n += check_int(log, r, 0);
	n += check_int(log, v.capacity, 20);
	n += check_int(log, v.size, 0);
	n += check_int(log, v.dt_size, sizeof(int));

	return -n;
}

test_case(tc_vector_init, "vector_init");

static int tc_vector_destroy(int log){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 20);

	n = 0;
	n += check_int(log, r, 0);

	vector_destroy(&v);

	n += check_int(log, v.capacity, 0);
	n += check_int(log, v.size, 0);
	n += check_int(log, v.dt_size, sizeof(int));

	return -n;
}

test_case(tc_vector_destroy, "vector_destroy");

static int tc_vector_add(int log){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 2);

	n = 0;
	n += check_int(log, r, 0);

	r = 10;
	r = vector_add(&v, &r);
	n += check_int(log, r, 0);
	n += check_int(log, v.capacity, 2);
	n += check_int(log, v.size, 1);

	r = 20;
	r = vector_add(&v, &r);
	n += check_int(log, r, 0);
	n += check_int(log, v.capacity, 2);
	n += check_int(log, v.size, 2);

	r = 30;
	r = vector_add(&v, &r);
	n += check_int(log, r, 0);
	n += check_int(log, v.capacity, 4);
	n += check_int(log, v.size, 3);

	n += check_int(log, ((int*)v.data)[0], 10);
	n += check_int(log, ((int*)v.data)[1], 20);
	n += check_int(log, ((int*)v.data)[2], 30);

	return -n;
}

test_case(tc_vector_add, "vector_add");

static int tc_vector_rm(int log){
	int n;
	int r;
	vector_t v;


	r = vector_init(&v, sizeof(int), 20);

	n = 0;
	n += check_int(log, r, 0);

	r = 10; r = vector_add(&v, &r);
	r = 20; r = vector_add(&v, &r);
	r = 30; r = vector_add(&v, &r);
	r = 40; r = vector_add(&v, &r);
	r = 50; r = vector_add(&v, &r);

	vector_rm(&v, 0);
	n += check_int(log, v.capacity, 20);
	n += check_int(log, v.size, 4);
	n += check_int(log, ((int*)v.data)[0], 20);
	n += check_int(log, ((int*)v.data)[1], 30);
	n += check_int(log, ((int*)v.data)[2], 40);
	n += check_int(log, ((int*)v.data)[3], 50);

	vector_rm(&v, 3);
	n += check_int(log, v.capacity, 20);
	n += check_int(log, v.size, 3);
	n += check_int(log, ((int*)v.data)[0], 20);
	n += check_int(log, ((int*)v.data)[1], 30);
	n += check_int(log, ((int*)v.data)[2], 40);

	vector_rm(&v, 1);
	n += check_int(log, v.capacity, 20);
	n += check_int(log, v.size, 2);
	n += check_int(log, ((int*)v.data)[0], 20);
	n += check_int(log, ((int*)v.data)[1], 40);

	return -n;
}

test_case(tc_vector_rm, "vector_rm");

