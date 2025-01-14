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
#define ARRAY(...)	{ __VA_ARGS__ }

#define DFLT_COMP	"default"
#define DFLT_I		1
#define DFLT_P		0x10
#define DFLT_IARR	{ 4, 2 }
#define DFLT_SARR	{ "f", "o", "o" }
#define DFLT_PARR	{ (void*)0xdead, (void*)0xbeef }

#define CHECK_DEV_SINGLE(_name, _compatible, _i, _p) \
	check_dev_single( \
		_name, \
		_compatible, \
		&((dev_single_payload_t){ \
			.i = _i, \
			.p = (void*)_p, \
		}) \
	)

#define CHECK_DEV_ARRAY(_name, _compatible, _i, _s, _p) \
	check_dev_array( \
		_name, \
		_compatible, \
		&((dev_array_payload_t){ \
			.i = _i, \
			.s = _s, \
			.p = _p, \
		}) \
	)


/* types */
typedef struct{
	uint8_t i;
	void *p;
	uint16_t iarr[2];
	char *sarr[3];
	void *aarr[2];
} dev_single_payload_t;

typedef struct{
	uint16_t i[2];
	char *s[3];
	void *p[2];
} dev_array_payload_t;


/* local/static prototypes */
static int checks(void);
static int check_dev_single(char const *name, char const *compatible, dev_single_payload_t *payload);
static int check_dev_array(char const *name, char const *compatible, dev_array_payload_t *payload);
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
	va_list arr;


	va_start(arr, fmt);
	vdprintf(fileno(log), fmt, arr);
	va_end(arr);
}


/* local functions */
static int checks(void){
	int r = 0;


	/* test included type */
	r |= check_memory("flash", (void*)0x10000000, 2097152);

	/* test type default attributes */
	r |= CHECK_DEV_SINGLE("def-0",	DFLT_COMP,	DFLT_I,	DFLT_P);
	r |= CHECK_DEV_SINGLE("def-1",	"def-1",	DFLT_I,	DFLT_P);
	r |= CHECK_DEV_SINGLE("def-2",	DFLT_COMP,	2,		DFLT_P);
	r |= CHECK_DEV_SINGLE("def-3",	DFLT_COMP,	DFLT_I,	0x20);
	r |= CHECK_DEV_SINGLE("def-4",	"def-4",	3,		0x30);

	r |= CHECK_DEV_ARRAY("def-5",	DFLT_COMP,	ARRAY(1, 2),	DFLT_SARR,				DFLT_PARR);
	r |= CHECK_DEV_ARRAY("def-6",	DFLT_COMP,	DFLT_IARR,		ARRAY("b", "a", "r"),	DFLT_PARR);
	r |= CHECK_DEV_ARRAY("def-7",	DFLT_COMP,	DFLT_IARR,		DFLT_SARR,				ARRAY((void*)0xbad, (void*)0xe1f));
	r |= CHECK_DEV_ARRAY("def-8",	"def-8",	ARRAY(3, 4),	ARRAY("n", "o", "p"),	ARRAY((void*)0x1, (void*)0x2));

	/* test attribute updates */
	r |= CHECK_DEV_SINGLE("upd-single",			"reset-add",	18,	0x2c);
	r |= CHECK_DEV_SINGLE("update-consumer",	"default",		17,	17);

	/* test arithmetics */
	r |= CHECK_DEV_SINGLE("donor-single",		"donation0",						42,	0xbe00);
	r |= CHECK_DEV_SINGLE("donor-child-single",	"donation1",						1,	0xef);
	r |= CHECK_DEV_SINGLE("arith-const-single",	"some-arith",						6,	0xbeef);
	r |= CHECK_DEV_SINGLE("ref-bare-single",	"donation0",						42,	0xbe00);
	r |= CHECK_DEV_SINGLE("arith-ref-single",	"pre,donation0-arith-donation1",	46,	0xbeef + 0x2);

	return -r;
}

static int check_dev_single(char const *name, char const *compatible, dev_single_payload_t *payload){
	int r = 0;
	devtree_device_t const *dev;


	TEST_LOG("test: dev=\"%s\"\n", name);
	dev = devtree_find_device_by_name(&__dt_device_root, name);
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, compatible);
	r |= TEST_INT_EQ(((dev_single_payload_t*)dev->payload)->i, payload->i);
	r |= TEST_PTR_EQ(((dev_single_payload_t*)dev->payload)->p, payload->p);

	return r;
}

static int check_dev_array(char const *name, char const *compatible, dev_array_payload_t *payload){
	int r = 0;
	devtree_device_t const *dev;


	TEST_LOG("test: dev=\"%s\"\n", name);
	dev = devtree_find_device_by_name(&__dt_device_root, name);
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, compatible);
	r |= TEST_INT_EQ(((dev_array_payload_t*)dev->payload)->i[0], payload->i[0]);
	r |= TEST_INT_EQ(((dev_array_payload_t*)dev->payload)->i[1], payload->i[1]);
	r |= TEST_STR_EQ(((dev_array_payload_t*)dev->payload)->s[0], payload->s[0]);
	r |= TEST_STR_EQ(((dev_array_payload_t*)dev->payload)->s[1], payload->s[1]);
	r |= TEST_STR_EQ(((dev_array_payload_t*)dev->payload)->s[2], payload->s[2]);
	r |= TEST_PTR_EQ(((dev_array_payload_t*)dev->payload)->p[0], payload->p[0]);
	r |= TEST_PTR_EQ(((dev_array_payload_t*)dev->payload)->p[1], payload->p[1]);

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
