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


/* types */
typedef struct{
	uint8_t i0,
			i1,
			i2;

	void *p0;
	char *s0,
		 *s1;
	void *p1;
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
	devtree_device_t const *dev;


	// arch updates
//	r |= TEST_INT_EQ(((devtree_arch_payload_t*)(__dt_arch.payload))->num_ints, 10);

	// base-dev attributes
	dev = devtree_find_device_by_name(&__dt_device_root, "base-dev");
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, "comp,base");
	r |= TEST_INT_EQ(((base_dev_payload_t*)dev->payload)->i0, 1);
	r |= TEST_INT_EQ(((base_dev_payload_t*)dev->payload)->i1, 12);
	r |= TEST_INT_EQ(((base_dev_payload_t*)dev->payload)->i2, 3);
	r |= TEST_PTR_EQ(((base_dev_payload_t*)dev->payload)->p0, 0x11);
	r |= TEST_PTR_EQ(((base_dev_payload_t*)dev->payload)->p1, 0x20);
	r |= TEST_STR_EQ(((base_dev_payload_t*)dev->payload)->s0, "first");
	r |= TEST_STR_EQ(((base_dev_payload_t*)dev->payload)->s1, "no longer second");

	// dev1 attributes
	dev = devtree_find_device_by_name(&__dt_device_root, "dev1");
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, "comp,base");
	r |= TEST_STR_EQ(((dev1_payload_t*)dev->payload)->s0, "no longer second");
	r |= TEST_STR_EQ(((dev1_payload_t*)dev->payload)->s1, "comp,dev2xx");
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i0, 2);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i1, 3);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i2, 270336);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i3, 6);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i4, 22);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i5, 32);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i6, 14);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i7, 4);
	r |= TEST_INT_EQ(((dev1_payload_t*)dev->payload)->i8, 6);
	r |= TEST_PTR_EQ(((dev1_payload_t*)dev->payload)->p0, 0x2a);

	// dev2 attributes
	dev = devtree_find_device_by_name(&__dt_device_root, "dev2");
	ASSERT_PTR_NEQ(dev, 0x0);

	r |= TEST_STR_EQ(dev->compatible, "comp,dev2");
	r |= TEST_INT_EQ(((dev2_payload_t*)dev->payload)->i0, 2);
	r |= TEST_INT_EQ(((dev2_payload_t*)dev->payload)->i1, 3);

	return -r;
}
