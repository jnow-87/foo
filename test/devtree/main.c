/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devtree.h>
#include <sys/errnums.h>
#include <sys/escape.h>
#include <sys/types.h>
#include <stdio.h>
#include <test/test.h>
#include "test.i.dts.h"


/* macros */
#define RESULT_EXT	".log"

#define CHECK_TEST_DEV(_name, _compatible, _i, _p) \
	check_test_dev( \
		_name, \
		_compatible, \
		&((test_dev_payload_t){ \
			.i = _i, \
			.p = (void*)_p, \
		}) \
	)


/* types */
typedef struct{
	uint8_t i;
	void *p;
} test_dev_payload_t;

typedef struct{
	uint8_t i0;
	void *p0;

	uint8_t i1,
			i2;

	void *p1;
	char *s0,
		 *s1;
	uint8_t l0[2];
} base_dev_payload_t;

typedef struct{
	char *s0,
		 *s1;
	uint32_t i0,
			 i1,
			 i2;
	void *p0;
	uint8_t i3,
			i4,
			i5,
			i6,
			i7,
			i8;
} dev1_payload_t;

typedef struct{
	uint8_t i0,
			i1;
} dev2_payload_t;


/* local/static prototypes */
static int checks(void);
static int check_test_dev(char const *name, char const *compatible, test_dev_payload_t *payload);
static int check_memory(char const *name, void *base, uint32_t size);


/* global variables */
errno_t errno;


/* static variables */
static FILE *log = 0x0;


/* global functions */
int main(int argc, char **argv){
	int r = 0;
	char log_file[strlen(argv[0]) + strlen(RESULT_EXT) + 1];


	/* init */
	strcpy(log_file, argv[0]);
	strcpy(log_file + strlen(argv[0]), RESULT_EXT);

	log = fopen(log_file, "w");
	stderr = log;

	if(log == 0x0){
		fprintf(stderr, "open log-file \"%s\" failed with %s\n", log_file, strerror(errno));
		return 1;
	}

	/* tests */
	r = checks();

	/* cleanup */
	fclose(log);

	if(r != 0)
		printf("devtree test " FG("failed", RED) " (cf. " FG("%s", PURPLE)")\n", log_file);

	return -r;
}

void test_log(char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	vdprintf(fileno(log), fmt, lst);
	va_end(lst);
}


/* local functions */
static int checks(void){
	int r = 0;


	/* test included type */
	r |= check_memory("flash", (void*)0x10000000, 2097152);

	/* test type default attributes */
	r |= CHECK_TEST_DEV("def-0", "default", 1, 0x10);
	r |= CHECK_TEST_DEV("def-1", "def-1", 1, 0x10);
	r |= CHECK_TEST_DEV("def-2", "default", 2, 0x10);
	r |= CHECK_TEST_DEV("def-3", "default", 1, 0x20);
	r |= CHECK_TEST_DEV("def-4", "def-4", 3, 0x30);

	/* test attribute updates */
	r |= CHECK_TEST_DEV("updates", "reset-add", 18, 0x2c);
	r |= CHECK_TEST_DEV("update-consumer", "default", 17, 17);

	/* test arithmetics */
	r |= CHECK_TEST_DEV("donor", "donation0", 42, 0xbe00);
	r |= CHECK_TEST_DEV("donor-child", "donation1", 1, 0xef);
	r |= CHECK_TEST_DEV("arith-const", "some-arith", 6, 0xbeef);
	r |= CHECK_TEST_DEV("ref-bare", "donation0", 42, 0xbe00);
	r |= CHECK_TEST_DEV("arith-ref", "pre,donation0-arith-donation1", 46, 0xbeef + 0x2);

	return -r;
}

static int check_test_dev(char const *name, char const *compatible, test_dev_payload_t *payload){
	int r = 0;
	devtree_device_t const *dev;


	TEST_LOG("test: dev=\"%s\"\n", name);
	dev = devtree_find_device_by_name(&__dt_device_root, name);
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, compatible);
	r |= TEST_INT_EQ(((test_dev_payload_t*)dev->payload)->i, payload->i);
	r |= TEST_PTR_EQ(((test_dev_payload_t*)dev->payload)->p, payload->p);

	return r;
}

static int check_memory(char const *name, void *base, uint32_t size){
	int r = 0;
	devtree_memory_t const *mem;


	TEST_LOG("test: memory=\"%s\"\n", name);
	mem = devtree_find_memory_by_name(&__dt_memory_root, name);
	ASSERT_PTR_NEQ(mem, 0x0);

	r |= TEST_PTR_EQ(mem->base, base);
	r |= TEST_INT_EQ(mem->size, size);

	return r;
}
