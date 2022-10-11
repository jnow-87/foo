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
	int r = 0;


	r |= TEST_IS(isalnum, "ajzAJZ079", "/:@[`{\t~");
	r |= TEST_IS(isalpha, "ajzAJZ", "/:@[`{\t~079");
	r |= TEST_IS(isblank, " \t", "\037!ajzAJZ079\010\012");
	r |= TEST_IS(iscntrl, "\001\033\037", " 7X~");
	r |= TEST_IS(isdigit, "079", "/:ajzAJZ\t");
	r |= TEST_IS(isgraph, "ajzAJZ079[}~\176", " \001\033\037\177");
	r |= TEST_IS(islower, "ajz", "AJZ079\t~");
	r |= TEST_IS(isprint, " ajzAJZ079[}~\176", "\001\033\037\177");
	r |= TEST_IS(ispunct, "!&:=/{~", "ajzAJZ079 \f\t\001\033\037");
	r |= TEST_IS(isspace, " \f\n\r\t\v", "ajz~");
	r |= TEST_IS(isupper, "AJZ", "ajz079\t~");
	r |= TEST_IS(isxdigit, "079acfACF", "/:gjzGJZ\t");

	return -r;
}

TEST(ctype_to){
	int r = 0;


	r |= TEST_TO(tolower, "AJZajz079~\001@[", "ajzajz079~\001@[");
	r |= TEST_TO(toupper, "ajzAJZ079~\001`{", "AJZAJZ079~\001`{");

	return -r;
}

static int test_is(int (*fp)(int), char const *valid, char const *invalid){
	int r = 0;


	r |= TEST_INT_NEQ(strlen(valid), 0);
	r |= TEST_INT_NEQ(strlen(invalid), 0);

	for(size_t i=0; i<strlen(valid); i++)
		r |= TEST_INT_EQ(fp(valid[i]), true);

	for(size_t i=0; i<strlen(invalid); i++)
		r |= TEST_INT_EQ(fp(invalid[i]), false);

	return -r;
}

static int test_to(int (*fp)(int), char const *s, char const *expect){
	int r = 0;


	r |= TEST_INT_EQ(strlen(s), strlen(expect));

	for(size_t i=0; i<strlen(s); i++)
		r |= TEST_INT_EQ(fp(s[i]), expect[i]);

	return -r;
}
