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
