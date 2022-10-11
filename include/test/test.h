/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TEST_H
#define TEST_H


#include <sys/stdarg.h>
#include <sys/compiler.h>
#include <sys/string.h>


/* macros */
// test definition
#ifndef TEST_TYPE
# define TEST_TYPE	""
#endif // TEST_TYPE

#define TEST_LONG(_name, _descr) \
	static int test_##_name(void); \
	static char const test_name_##_name[] __used = #_name; \
	static char const test_descr_##_name[] __used = _descr; \
	\
	static test_t test_cfg_##_name __linker_array("tests_"TEST_TYPE) = { \
		.name = test_name_##_name, \
		.descr = test_descr_##_name, \
		.call = test_##_name, \
	}; \
	\
	static int test_##_name(void)

#define TEST(name)	TEST_LONG(name, "")

// test traversal
#define _test_for_each(test, type) \
	extern test_t __start_tests_##type[], \
				  __stop_tests_##type[]; \
	\
	\
	for(test=__start_tests_##type; test!=__stop_tests_##type; test++)

#define test_for_each_type(test, type)	_test_for_each(test, type)
#define test_for_each(test)				test_for_each_type(test, )

// logging
#ifndef TEST_NO_LOG
# define TEST_LOG(fmt, ...) \
	test_log("%s:%d:%s():\t" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#else
# define TEST_LOG(fmt, ...)
#endif // TEST_NO_LOG


// invariant checks
/**
 * \brief	basic macro for invariant checks and test log update
 *
 * \param	eq		test for equality (1) or not (0)
 * \param	cmp		compare method with the following interface
 * 						int cmp(result, reference, n)
 * 							compare the first n bytes of result
 * 							and reference, n = 0 is ignored
 *
 * 							return 1 if result and reference
 * 							match and 0 otherwise
 *
 * \param	n		number of bytes to compare - passed on to cmp()
 * \param	fmt		printf specifier for printing the result of
 * 					expr and ref
 *
 * \param	expr	expression to check
 * \param	ref		expected result of expr
 *
 * \return	0 if the result of expr matches ref
 * 			1 otherwise
 */
#define _TEST_EVAL(eq, cmp, n, fmt, expr, ref)({ \
	typeof(ref) _result = expr, \
				_ref = ref; \
	int _eq = cmp(_result, _ref, n), \
		_passed = (_eq == eq); \
	\
	\
	TEST_LOG("(%s = " fmt ") %s " fmt " [%s]\n", \
		#expr, \
		_result, \
		(((char *[]){ "!=", "==" })[_eq]), \
		_ref, \
		(((char *[]){ "failed", "passed" })[_passed]) \
	); \
	\
	(_passed ? 0 : 1); \
})

#define TEST_CMP(res, ref, n)				((res) == (ref))
#define TEST_CMP_STR(res, ref, n)			(strcmp(res, ref) == 0)
#define TEST_CMP_STRN(res, ref, n)			(strncmp(res, ref, n) == 0)

#define TEST_EVAL(eq, fmt, expr, ref)		_TEST_EVAL(eq, TEST_CMP, 0, fmt, expr, ref)
#define TEST_EVAL_STR(eq, expr, ref)		_TEST_EVAL(eq, TEST_CMP_STR, 0, "%s", expr, (char const*)ref)
#define TEST_EVAL_STRN(eq, expr, ref, n)	_TEST_EVAL(eq, TEST_CMP_STRN, n, "%s", expr, (char const*)ref)

#define TEST_INT_EQ(expr, ref)				TEST_EVAL(1, "%ld", expr, (long int)ref)
#define TEST_INT_NEQ(expr, ref)				TEST_EVAL(0, "%ld", expr, (long int)ref)
#define TEST_PTR_EQ(expr, ref)				TEST_EVAL(1, "%p", expr, (void const *)(ref))
#define TEST_PTR_NEQ(expr, ref)				TEST_EVAL(0, "%p", expr, (void const *)(ref))
#define TEST_STR_EQ(expr, ref)				TEST_EVAL_STR(1, expr, ref)
#define TEST_STR_NEQ(expr, ref)				TEST_EVAL_STR(0, expr, ref)
#define TEST_STRN_EQ(expr, ref, n)			TEST_EVAL_STRN(1, expr, ref, n)
#define TEST_STRN_NEQ(expr, ref, n)			TEST_EVAL_STRN(0, expr, ref, n)

#define _ASSERT_EVAL(eq, cmp, n, fmt, expr, ref){ \
	if(_TEST_EVAL(eq, cmp, n, fmt, expr, ref) != 0) \
		return -1; \
}

#define ASSERT_EVAL(eq, fmt, expr, ref)		_ASSERT_EVAL(eq, TEST_CMP, 0, fmt, expr, ref)
#define ASSERT_EVAL_STR(eq, expr, ref)		_ASSERT_EVAL(eq, TEST_CMP_STR, 0, "%s", expr, (char const*)ref)
#define ASSERT_EVAL_STRN(eq, expr, ref, n)	_ASSERT_EVAL(eq, TEST_CMP_STRN, n, "%s", expr, (char const*)ref)

#define ASSERT_INT_EQ(expr, ref)			ASSERT_EVAL(1, "%ld", expr, (long int)ref)
#define ASSERT_INT_NEQ(expr, ref)			ASSERT_EVAL(0, "%ld", expr, (long int)ref)
#define ASSERT_PTR_EQ(expr, ref)			ASSERT_EVAL(1, "%p", expr, (void const *)(ref))
#define ASSERT_PTR_NEQ(expr, ref)			ASSERT_EVAL(0, "%p", expr, (void const *)(ref))
#define ASSERT_STR_EQ(expr, ref)			ASSERT_EVAL_STR(1, expr, ref)
#define ASSERT_STR_NEQ(expr, ref)			ASSERT_EVAL_STR(0, expr, ref)
#define ASSERT_STRN_EQ(expr, ref, n)		ASSERT_EVAL_STRN(1, expr, ref, n)
#define ASSERT_STRN_NEQ(expr, ref, n)		ASSERT_EVAL_STRN(0, expr, ref, n)


/* types */
typedef struct{
	char const *name,
			   *descr;

	int (*call)(void);
} test_t;


/* prototypes */
#ifndef TEST_NO_LOG
void test_log(char const *fmt, ...);
#endif // TEST_NO_LOG


#endif // TEST_H
