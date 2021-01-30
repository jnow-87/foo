/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TESTCASE_H
#define TESTCASE_H


/* brickos header */
#include <sys/compiler.h>

/* host header */
#include <unistd.h>


/* macros */
#define TEST(_name, _desc) \
	static int test_##_name(void); \
	static char const test_case_desc_##_name[]  __used = _desc; \
	static test_case_t test_case_##_name __linker_array("testcases") = { \
		.hdlr = test_##_name, \
		.desc = test_case_desc_##_name, \
	}; \
	\
	static int test_##_name(void)

#define TEST_LOG(fmt, ...)	test_log(__FILE__ ":%d " fmt, __LINE__, ##__VA_ARGS__)

#define RES_OP(expr)		(((char *[]){ "!=", "==" })[expr])

#define CHECK_INT(expr, ref)({ \
	typeof(ref) _res; \
	\
	\
	_res = expr; \
	TEST_LOG("%s: res(%d) %s ref(%d)\n", #expr, _res, RES_OP(_res == (ref)), ref); \
	\
	(_res == (ref)) ? 0 : 1; \
})

#define CHECK_PTR(expr, ref)({ \
	void const *_res; \
	\
	\
	_res = expr; \
	TEST_LOG("%s: res(%p) %s ref(%p)\n", #expr, _res, RES_OP(_res == (ref)), (void*)ref); \
	\
	(_res == (ref)) ? 0 : 1; \
})

#define CHECK_STR(expr, s, ref)({ \
	expr; \
	TEST_LOG("%s: res('%s') %s ref('%s')\n", #expr, (char*)s, RES_OP(strcmp(s, ref) == 0), (char*)ref); \
	\
	(strcmp(s, ref) == 0) ? 0 : 1; \
})

#define CHECK_STRN(expr, s, ref, n)({ \
	expr; \
	TEST_LOG("%s: res('%s') %s ref('%s')\n", #expr, (char*)s, RES_OP(strncmp(s, ref, n) == 0), (char*)ref); \
	\
	(strncmp(s, ref, n) == 0) ? 0 : 1; \
})


/* types */
typedef int (*tc_hdlr_t)(void);

typedef struct{
	tc_hdlr_t hdlr;
	char const *desc;
} test_case_t;


/* prototypes */
int test_init(char const *log_name);
void test_close(void);

void test_log(char const *fmt, ...);


#endif // TESTCASE_H
