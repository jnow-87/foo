/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <test/test.h>


/* local functions */
TEST(strcmp){
	int n;


	n = 0;

	n += TEST_INT_EQ(memcmp("foo", "foo", 4), 0);
	n += TEST_INT_EQ(memcmp("foo", "bar", 4), 1);
	n += TEST_INT_EQ(memcmp("bar", "foo", 4), -1);

	n += TEST_INT_EQ(strcmp("foo", "foo"), 0);
	n += TEST_INT_EQ(strcmp("foo", "bar"), 1);
	n += TEST_INT_EQ(strcmp("bar", "foo"), -1);
	n += TEST_INT_EQ(strcmp("foo", "foobar"), -1);

	n += TEST_INT_EQ(strncmp("foobar", "foo", 3), 0);
	n += TEST_INT_EQ(strncmp("foobar", "barbar", 3), 1);
	n += TEST_INT_EQ(strncmp("barbar", "foobar", 3), -1);

	return -n;
}

TEST(strlen){
	int n;


	n = 0;

	n += TEST_INT_EQ(strlen(""), 0);
	n += TEST_INT_EQ(strlen("foo"), 3);

	return -n;
}

TEST(strcnt){
	int n;


	n = 0;

	n += TEST_INT_EQ(strcnt("", '%'), 0);
	n += TEST_INT_EQ(strcnt("foo", '%'), 0);
	n += TEST_INT_EQ(strcnt("%foo", '%'), 1);
	n += TEST_INT_EQ(strcnt("foo%", '%'), 1);
	n += TEST_INT_EQ(strcnt("%bar%foo%", '%'), 3);

	return -n;
}

TEST(isoneof){
	int n;


	n = 0;

	n += TEST_INT_EQ(isoneof(' ', 0x0), false);
	n += TEST_INT_EQ(isoneof('a', ""), false);
	n += TEST_INT_EQ(isoneof('a', "bcde"), false);
	n += TEST_INT_EQ(isoneof('a', "abcde"), true);
	n += TEST_INT_EQ(isoneof('a', "bcade"), true);

	return -n;
}

TEST(strcpy){
	int n;
	char d[5];
	char s[] = "foo";


	n = 0;
	d[4] = 0;

	memset(d, 'x', 4);
	n += TEST_STRN_EQ(memset(d, 'y', 4), "yyyy", 4);

	memset(d, 'x', 4);
	n += TEST_STRN_EQ(memcpy(d, s, 4), s, 4);

	strcpy(s, "abc");
	n += TEST_STR_EQ(memcpy(s + 1, s, 2), "aab");

	memset(d, 'x', 4);
	n += TEST_STR_EQ(strcpy(d, s), s);

	memset(d, 'x', 4);
	n += TEST_STRN_EQ(strncpy(d, s, 2), s, 2);

	return -n;
}

TEST(memscan){
	int n;
	char s[] = "foobar";


	n = 0;

	n += TEST_PTR_EQ(memscan(s, 'f', strlen(s)), s + 0);
	n += TEST_PTR_EQ(memscan(s, 'b', strlen(s)), s + 3);
	n += TEST_PTR_EQ(memscan(s, 'r', strlen(s)), s + 5);
	n += TEST_PTR_EQ(memscan(s, 'x', strlen(s)), 0x0);

	return -n;
}

TEST(memnscan){
	int n;
	size_t (*fp_array[10])(char const *);
	size_t (*fp_zero)(char const *);
	size_t (*fp_nonzero)(char const *);


	n = 0;
	fp_zero = 0x0;
	fp_nonzero = strlen;

	memset(fp_array, 0x0, sizeof(fp_array));
	n += TEST_PTR_EQ(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 0);
	n += TEST_PTR_EQ(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), 0x0);

	fp_array[0] = strlen;
	n += TEST_PTR_EQ(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 1);
	n += TEST_PTR_EQ(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), fp_array + 0);

	return -n;
}

TEST(strerror){
	int n;


	n = 0;
	n += TEST_STR_EQ(strerror(E_OK), "Success");
	n += TEST_STR_EQ(strerror(E_UNKNOWN), "Unknown");
	n += TEST_STR_EQ(strerror(0xfe), "Unknown error 0xfe");
	n += TEST_STR_EQ(strerror(0xfefe), "Error string too short to display errno");

	return -n;
}

TEST(itoa){
	int n;
	char s[16];


	n = 0;

	/* different base values */
	n += TEST_STR_EQ(itoa(10, 8, s, 15), "12");
	n += TEST_STR_EQ(itoa(10, 10, s, 15), "10");
	n += TEST_STR_EQ(itoa(10, 16, s, 15), "a");

	/* too large number */
	n += TEST_PTR_EQ(itoa(255, 10, s, 2), 0x0);

	return -n;
}

TEST(strtol){
	int n;
	char *s;


	n = 0;

	/* different base values */
	n += TEST_INT_EQ(strtol("10", 0x0, 10), 10);
	n += TEST_INT_EQ(strtol("10", 0x0, 8), 8);
	n += TEST_INT_EQ(strtol("deAdbEEF", 0x0, 16), 3735928559);

	/* auto detected base */
	n += TEST_INT_EQ(strtol("10", 0x0, 0), 10);
	n += TEST_INT_EQ(strtol("0x10", 0x0, 0), 16);
	n += TEST_INT_EQ(strtol("011", 0x0, 0), 9);

	/* signs */
	n += TEST_INT_EQ(strtol("0xf", 0x0, 16), 15);
	n += TEST_INT_EQ(strtol("+0xf", 0x0, 16), 15);
	n += TEST_INT_EQ(strtol("-0xf", 0x0, 16), -15);

	/* partial match */
	n += TEST_INT_EQ(strtol("10f", 0x0, 10), 10);
	n += TEST_INT_EQ(strtol("10f", 0x0, 16), 271);

	/* use endp */
	strtol("10f", &s, 10);
	n += TEST_STR_EQ(s, "f");

	strtol("0xfgo", &s, 0);
	n += TEST_STR_EQ(s, "go");

	/* invalid inputs */
	n += TEST_INT_EQ(strtol(" 0xfe", &s, 8), 0);
	n += TEST_INT_EQ(strtol("0xfe", &s, 8), 0);
	n += TEST_STR_EQ(s, "xfe");

	/* atoi */
	n += TEST_INT_EQ(atoi("10"), 10);
	n += TEST_INT_EQ(atoi("0xee"), 0);

	return -n;
}
