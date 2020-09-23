/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <testcase.h>


/* local functions */
TEST(strcmp, "strcmp"){
	int n;


	n = 0;

	n += CHECK_INT(memcmp("foo", "foo", 4), 0);
	n += CHECK_INT(memcmp("foo", "bar", 4), 1);
	n += CHECK_INT(memcmp("bar", "foo", 4), -1);

	n += CHECK_INT(strcmp("foo", "foo"), 0);
	n += CHECK_INT(strcmp("foo", "bar"), 1);
	n += CHECK_INT(strcmp("bar", "foo"), -1);
	n += CHECK_INT(strcmp("foo", "foobar"), -1);

	n += CHECK_INT(strncmp("foobar", "foo", 3), 0);
	n += CHECK_INT(strncmp("foobar", "barbar", 3), 1);
	n += CHECK_INT(strncmp("barbar", "foobar", 3), -1);

	return -n;
}

TEST(strlen, "strlen"){
	int n;


	n = 0;

	n += CHECK_INT(strlen(""), 0);
	n += CHECK_INT(strlen("foo"), 3);

	return -n;
}

TEST(strcnt, "strcnt"){
	int n;


	n = 0;

	n += CHECK_INT(strcnt("", '%'), 0);
	n += CHECK_INT(strcnt("foo", '%'), 0);
	n += CHECK_INT(strcnt("%foo", '%'), 1);
	n += CHECK_INT(strcnt("foo%", '%'), 1);
	n += CHECK_INT(strcnt("%bar%foo%", '%'), 3);

	return -n;
}

TEST(isoneof, "isoneof"){
	int n;


	n = 0;

	n += CHECK_INT(isoneof(' ', 0x0), false);
	n += CHECK_INT(isoneof('a', ""), false);
	n += CHECK_INT(isoneof('a', "bcde"), false);
	n += CHECK_INT(isoneof('a', "abcde"), true);
	n += CHECK_INT(isoneof('a', "bcade"), true);

	return -n;
}

TEST(strcpy, "strcpy"){
	int n;
	char d[5];
	char s[] = "foo";


	n = 0;
	d[4] = 0;

	memset(d, 'x', 4);
	n += CHECK_STRN(memset(d, 'y', 4), d, "yyyy", 4);

	memset(d, 'x', 4);
	n += CHECK_STRN(memcpy(d, s, 4), d, s, 4);

	strcpy(s, "abc");
	n += CHECK_STR(memcpy(s + 1, s, 2), s, "aab");

	memset(d, 'x', 4);
	n += CHECK_STR(strcpy(d, s), d, s);

	memset(d, 'x', 4);
	n += CHECK_STRN(strncpy(d, s, 2), d, s, 2);

	return -n;
}

TEST(memscan, "memscan"){
	int n;
	char s[] = "foobar";


	n = 0;

	n += CHECK_PTR(memscan(s, 'f', strlen(s)), s + 0);
	n += CHECK_PTR(memscan(s, 'b', strlen(s)), s + 3);
	n += CHECK_PTR(memscan(s, 'r', strlen(s)), s + 5);
	n += CHECK_PTR(memscan(s, 'x', strlen(s)), 0x0);

	return -n;
}

TEST(memnscan, "memnscan"){
	int n;
	size_t (*fp_array[10])(char const *);
	size_t (*fp_zero)(char const *);
	size_t (*fp_nonzero)(char const *);


	n = 0;
	fp_zero = 0x0;
	fp_nonzero = strlen;

	memset(fp_array, 0x0, sizeof(fp_array));
	n += CHECK_PTR(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 0);
	n += CHECK_PTR(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), 0x0);

	fp_array[0] = strlen;
	n += CHECK_PTR(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 1);
	n += CHECK_PTR(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), fp_array + 0);

	return -n;
}

TEST(strerror, "strerror"){
	int n;


	n = 0;
	n += CHECK_STR(, strerror(E_OK), "Success");
	n += CHECK_STR(, strerror(E_UNKNOWN), "Unknown");
	n += CHECK_STR(, strerror(0xfe), "Unknown error 0xfe");
	n += CHECK_STR(, strerror(0xfefe), "Error string too short to display errno");

	return -n;
}

TEST(itoa, "itoa"){
	int n;
	char s[16];


	n = 0;

	/* different base values */
	n += CHECK_STR(, itoa(10, 8, s, 15), "12");
	n += CHECK_STR(, itoa(10, 10, s, 15), "10");
	n += CHECK_STR(, itoa(10, 16, s, 15), "a");

	/* too large number */
	n += CHECK_PTR(itoa(255, 10, s, 2), 0x0);

	return -n;
}

TEST(strtol, "strtol"){
	int n;
	char *s;


	n = 0;

	/* different base values */
	n += CHECK_INT(strtol("10", 0x0, 10), 10);
	n += CHECK_INT(strtol("10", 0x0, 8), 8);
	n += CHECK_INT(strtol("deAdbEEF", 0x0, 16), 3735928559);

	/* auto detected base */
	n += CHECK_INT(strtol("10", 0x0, 0), 10);
	n += CHECK_INT(strtol("0x10", 0x0, 0), 16);
	n += CHECK_INT(strtol("011", 0x0, 0), 9);

	/* signs */
	n += CHECK_INT(strtol("0xf", 0x0, 16), 15);
	n += CHECK_INT(strtol("+0xf", 0x0, 16), 15);
	n += CHECK_INT(strtol("-0xf", 0x0, 16), -15);

	/* partial match */
	n += CHECK_INT(strtol("10f", 0x0, 10), 10);
	n += CHECK_INT(strtol("10f", 0x0, 16), 271);

	/* use endp */
	n += CHECK_STR(strtol("10f", &s, 10), s, "f");
	n += CHECK_STR(strtol("0xfgo", &s, 0), s, "go");

	/* invalid inputs */
	n += CHECK_INT(strtol(" 0xfe", &s, 8), 0);
	n += CHECK_INT(strtol("0xfe", &s, 8), 0);
	n += CHECK_STR(, s, "xfe");

	/* atoi */
	n += CHECK_INT(atoi("10"), 10);
	n += CHECK_INT(atoi("0xee"), 0);

	return -n;
}
