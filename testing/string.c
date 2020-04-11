/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>
#include <testing/testcase.h>


/* local functions */
static int tc_strcmp(int log){
	unsigned int n;


	n = 0;

	n += check_int(log, memcmp("foo", "foo", 4), 0);
	n += check_int(log, memcmp("foo", "bar", 4), 1);
	n += check_int(log, memcmp("bar", "foo", 4), -1);

	n += check_int(log, strcmp("foo", "foo"), 0);
	n += check_int(log, strcmp("foo", "bar"), 1);
	n += check_int(log, strcmp("bar", "foo"), -1);
	n += check_int(log, strcmp("foo", "foobar"), -1);

	n += check_int(log, strncmp("foobar", "foo", 3), 0);
	n += check_int(log, strncmp("foobar", "barbar", 3), 1);
	n += check_int(log, strncmp("barbar", "foobar", 3), -1);

	return -n;
}

test_case(tc_strcmp, "strcmp");

static int tc_strlen(int log){
	unsigned int n;


	n = 0;

	n += check_int(log, strlen(""), 0);
	n += check_int(log, strlen("foo"), 3);

	return -n;
}

test_case(tc_strlen, "strlen");

static int tc_strcnt(int log){
	unsigned int n;


	n = 0;

	n += check_int(log, strcnt("", '%'), 0);
	n += check_int(log, strcnt("foo", '%'), 0);
	n += check_int(log, strcnt("%foo", '%'), 1);
	n += check_int(log, strcnt("foo%", '%'), 1);
	n += check_int(log, strcnt("%bar%foo%", '%'), 3);

	return -n;
}

test_case(tc_strcnt, "strcnt");

static int tc_isoneof(int log){
	unsigned int n;


	n = 0;

	n += check_int(log, isoneof('a', ""), false);
	n += check_int(log, isoneof('a', "bcde"), false);
	n += check_int(log, isoneof('a', "abcde"), true);
	n += check_int(log, isoneof('a', "bcade"), true);

	return -n;
}

test_case(tc_isoneof, "isoneof");

static int tc_strcpy(int log){
	unsigned int n;
	char d[5];
	char s[] = "foo";


	n = 0;
	d[4] = 0;

	memset(d, 'x', 4);
	n += check_strn(log, memset(d, 'y', 4), d, "yyyy", 4);

	memset(d, 'x', 4);
	n += check_strn(log, memcpy(d, s, 4), d, s, 4);

	strcpy(s, "abc");
	n += check_str(log, memcpy(s + 1, s, 2), s, "aab");

	memset(d, 'x', 4);
	n += check_str(log, strcpy(d, s), d, s);

	memset(d, 'x', 4);
	n += check_strn(log, strncpy(d, s, 2), d, s, 2);

	return -n;
}

test_case(tc_strcpy, "strcpy");

static int tc_memscan(int log){
	unsigned int n;
	char s[] = "foobar";


	n = 0;

	n += check_ptr(log, memscan(s, 'f', strlen(s)), s + 0);
	n += check_ptr(log, memscan(s, 'b', strlen(s)), s + 3);
	n += check_ptr(log, memscan(s, 'r', strlen(s)), s + 5);
	n += check_ptr(log, memscan(s, 'x', strlen(s)), 0x0);

	return -n;
}

test_case(tc_memscan, "memscan");

static int tc_memnscan(int log){
	unsigned int n;
	int (*fp_array[10])(int);
	int (*fp_zero)(int);
	int (*fp_nonzero)(int);


	n = 0;
	fp_zero = 0x0;
	fp_nonzero = tc_memnscan;

	memset(fp_array, 0x0, sizeof(fp_array));
	n += check_ptr(log, memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 0);
	n += check_ptr(log, memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), 0x0);

	fp_array[0] = tc_memnscan;
	n += check_ptr(log, memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 1);
	n += check_ptr(log, memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), fp_array + 0);

	return -n;
}

test_case(tc_memnscan, "memnscan");
