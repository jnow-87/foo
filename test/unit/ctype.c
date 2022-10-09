/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ctype.h>
#include <test/test.h>
#include <sys/compiler.h>


/* macros */
#define TEST_IS(func, valid, invalid)({ \
	TEST_LOG("%s(): valid = \"%s\", invalid = \"%s\"\n", STR(func), valid, invalid); \
	test_is(func, valid, invalid); \
})

#define TEST_TO(func, s, expect)({ \
	TEST_LOG("%s(): s = \"%s\", expect = \"%s\"\n", STR(func), s, expect); \
	test_to(func, s, expect); \
})


/* local/static prototypes */
static int test_is(int (*fp)(int), char const *valid, char const *invalid);
static int test_to(int (*fp)(int), char const *s, char const *expect);


/* local functions */
TEST(ctype_is){
	int n = 0;


	n |= TEST_IS(isalnum, "ajzAJZ079", "/:@[`{\t~");
	n |= TEST_IS(isalpha, "ajzAJZ", "/:@[`{\t~079");
	n |= TEST_IS(isblank, " \t", "\037!ajzAJZ079\010\012");
	n |= TEST_IS(iscntrl, "\001\033\037", " 7X~");
	n |= TEST_IS(isdigit, "079", "/:ajzAJZ\t");
	n |= TEST_IS(isgraph, "ajzAJZ079[}~\176", " \001\033\037\177");
	n |= TEST_IS(islower, "ajz", "AJZ079\t~");
	n |= TEST_IS(isprint, " ajzAJZ079[}~\176", "\001\033\037\177");
	n |= TEST_IS(ispunct, "!&:=/{~", "ajzAJZ079 \f\t\001\033\037");
	n |= TEST_IS(isspace, " \f\n\r\t\v", "ajz~");
	n |= TEST_IS(isupper, "AJZ", "ajz079\t~");
	n |= TEST_IS(isxdigit, "079acfACF", "/:gjzGJZ\t");

	return -n;
}

TEST(ctype_to){
	int n = 0;


	n |= TEST_TO(tolower, "AJZajz079~\001@[", "ajzajz079~\001@[");
	n |= TEST_TO(toupper, "ajzAJZ079~\001`{", "AJZAJZ079~\001`{");

	return -n;
}

static int test_is(int (*fp)(int), char const *valid, char const *invalid){
	int n = 0;


	n |= TEST_INT_NEQ(strlen(valid), 0);
	n |= TEST_INT_NEQ(strlen(invalid), 0);

	for(size_t i=0; i<strlen(valid); i++)
		n |= TEST_INT_EQ(fp(valid[i]), true);

	for(size_t i=0; i<strlen(invalid); i++)
		n |= TEST_INT_EQ(fp(invalid[i]), false);

	return -n;
}

static int test_to(int (*fp)(int), char const *s, char const *expect){
	int n = 0;


	n |= TEST_INT_EQ(strlen(s), strlen(expect));

	for(size_t i=0; i<strlen(s); i++)
		n |= TEST_INT_EQ(fp(s[i]), expect[i]);

	return -n;
}
