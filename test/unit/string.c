/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/ctype.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <test/test.h>


/* types */
typedef struct{
	void (*foo)(void);
	void (*bar)(void);
} cb_t;


/* local/static prototypes */
static int test_deesc(char const *s, char const *ref);
static int esc_resolve(int c);


/* local functions */
TEST(strcmp){
	int r = 0;


	r += TEST_INT_EQ(memcmp("foo", "foo", 4), 0);
	r += TEST_INT_EQ(memcmp("foo", "bar", 4), 1);
	r += TEST_INT_EQ(memcmp("bar", "foo", 4), -1);

	r += TEST_INT_EQ(strcmp("foo", "foo"), 0);
	r += TEST_INT_EQ(strcmp("foo", "bar"), 1);
	r += TEST_INT_EQ(strcmp("bar", "foo"), -1);
	r += TEST_INT_EQ(strcmp("foo", "foobar"), -1);

	r += TEST_INT_EQ(strncmp("foobar", "foo", 3), 0);
	r += TEST_INT_EQ(strncmp("foobar", "barbar", 3), 1);
	r += TEST_INT_EQ(strncmp("barbar", "foobar", 3), -1);

	return -r;
}

TEST(strlen){
	int r = 0;


	r += TEST_INT_EQ(strlen(""), 0);
	r += TEST_INT_EQ(strlen("foo"), 3);

	return -r;
}

TEST(strcnt){
	int r = 0;


	r += TEST_INT_EQ(strcnt("", '%'), 0);
	r += TEST_INT_EQ(strcnt("foo", '%'), 0);
	r += TEST_INT_EQ(strcnt("%foo", '%'), 1);
	r += TEST_INT_EQ(strcnt("foo%", '%'), 1);
	r += TEST_INT_EQ(strcnt("%bar%foo%", '%'), 3);

	return -r;
}

TEST(strcat){
	int r = 0;
	char s[16];


	strcpy(s, "foo");

	s[3] = 0; r += TEST_STR_EQ(strcat(s, "bar"), s);
	s[3] = 0; r += TEST_STR_EQ(strcat(s, "bar"), "foobar");
	s[3] = 0; r += TEST_STR_EQ(strcat(s, s), "foofoo");
	s[3] = 0; r += TEST_STR_EQ(strcat(s, ""), "foo");
	s[0] = 0; r += TEST_STR_EQ(strcat(s, "foo"), "foo");

	strcpy(s, "foo");

	s[3] = 0; r += TEST_STR_EQ(strncat(s, "bar", 2), s);
	s[3] = 0; r += TEST_STR_EQ(strncat(s, "bar", 2), "fooba");
	s[3] = 0; r += TEST_STR_EQ(strncat(s, "bar", 5), "foobar");
	s[3] = 0; r += TEST_STR_EQ(strncat(s, s, 2), "foofo");
	s[3] = 0; r += TEST_STR_EQ(strncat(s, "", 2), "foo");
	s[0] = 0; r += TEST_STR_EQ(strncat(s, "foo", 2), "fo");

	return -r;
}

TEST(strchr){
	int r = 0;
	char s[32];


	strcpy(s, "1590 ahz\tAHZ1590 ahz\tAHZ");

	r += TEST_STR_EQ(strchr(s, '1'), s + 0);
	r += TEST_STR_EQ(strchr(s, ' '), s + 4);
	r += TEST_STR_EQ(strchr(s, '9'), s + 2);
	r += TEST_STR_EQ(strchr(s, 'a'), s + 5);
	r += TEST_STR_EQ(strchr(s, 'A'), s + 9);
	r += TEST_STR_EQ(strchr(s, '\t'), s + 8);
	r += TEST_STR_EQ(strchr(s, '0'), s + 3);
	r += TEST_STR_EQ(strchr(s, 'z'), s + 7);
	r += TEST_STR_EQ(strchr(s, 'Z'), s + 11);
	r += TEST_STR_EQ(strchr(s, '5'), s + 1);
	r += TEST_STR_EQ(strchr(s, 'h'), s + 6);
	r += TEST_STR_EQ(strchr(s, 'H'), s + 10);
	r += TEST_STR_EQ(strchr(s, 0), s + 24);
	r += TEST_PTR_EQ(strchr(s, 'x'), 0x0);

	return -r;
}

TEST(strrchr){
	int r = 0;
	char s[32];


	memset(s, '1', 32);
	strcpy(s, "1590 ahz\tAHZ1590 ahz\tAHZ");

	r += TEST_STR_EQ(strrchr(s, '1'), s + 12);
	r += TEST_STR_EQ(strrchr(s, ' '), s + 16);
	r += TEST_STR_EQ(strrchr(s, '9'), s + 14);
	r += TEST_STR_EQ(strrchr(s, 'a'), s + 17);
	r += TEST_STR_EQ(strrchr(s, 'A'), s + 21);
	r += TEST_STR_EQ(strrchr(s, '\t'), s + 20);
	r += TEST_STR_EQ(strrchr(s, '0'), s + 15);
	r += TEST_STR_EQ(strrchr(s, 'z'), s + 19);
	r += TEST_STR_EQ(strrchr(s, 'Z'), s + 23);
	r += TEST_STR_EQ(strrchr(s, '5'), s + 13);
	r += TEST_STR_EQ(strrchr(s, 'h'), s + 18);
	r += TEST_STR_EQ(strrchr(s, 'H'), s + 22);
	r += TEST_STR_EQ(strrchr(s, 0), s + 24);
	r += TEST_PTR_EQ(strrchr(s, 'x'), 0x0);

	return -r;
}

TEST(strpchr){
	int r = 0;
	char s[32];


	strcpy(s, "1590 ahz\tAHZ1590 ahz\tAHZ");

	r += TEST_STR_EQ(strpchr(s, isdigit), s + 0);
	r += TEST_STR_EQ(strpchr(s, isblank), s + 4);
	r += TEST_STR_EQ(strpchr(s, isalpha), s + 5);
	r += TEST_STR_EQ(strpchr(s, isupper), s + 9);

	strcpy(s, "noctrl");
	r += TEST_STR_EQ(strpchr(s, iscntrl), s + strlen(s));

	return -r;
}

TEST(isoneof){
	int r = 0;


	r += TEST_INT_EQ(isoneof(' ', 0x0), false);
	r += TEST_INT_EQ(isoneof('a', ""), false);
	r += TEST_INT_EQ(isoneof('a', "bcde"), false);
	r += TEST_INT_EQ(isoneof('a', "abcde"), true);
	r += TEST_INT_EQ(isoneof('a', "bcade"), true);

	return -r;
}

TEST(strcpy){
	int r = 0;
	char d[5];
	char s[] = "foo";


	d[4] = 0;

	memset(d, 'x', 4);
	r += TEST_STRN_EQ(memset(d, 'y', 4), "yyyy", 4);

	memset(d, 'x', 4);
	r += TEST_STRN_EQ(memcpy(d, s, 4), s, 4);

	strcpy(s, "abc");
	r += TEST_STR_EQ(memcpy(s + 1, s, 2), "aab");

	memset(d, 'x', 4);
	r += TEST_STR_EQ(strcpy(d, s), s);

	memset(d, 'x', 4);
	r += TEST_STRN_EQ(strncpy(d, s, 2), s, 2);

	return -r;
}

TEST(memscan){
	int r = 0;
	char s[] = "foobar";


	r += TEST_PTR_EQ(memscan(s, 'f', strlen(s)), s + 0);
	r += TEST_PTR_EQ(memscan(s, 'b', strlen(s)), s + 3);
	r += TEST_PTR_EQ(memscan(s, 'r', strlen(s)), s + 5);
	r += TEST_PTR_EQ(memscan(s, 'x', strlen(s)), 0x0);

	return -r;
}

TEST(memnscan){
	int r = 0;
	size_t (*fp_zero)(char const *) = 0x0;
	size_t (*fp_nonzero)(char const *) = strlen;
	size_t (*fp_array[10])(char const *);


	memset(fp_array, 0x0, sizeof(fp_array));
	r += TEST_PTR_EQ(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 0);
	r += TEST_PTR_EQ(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), 0x0);

	fp_array[0] = strlen;
	r += TEST_PTR_EQ(memnscan(fp_array, &fp_zero, 10, sizeof(fp_zero)), fp_array + 1);
	r += TEST_PTR_EQ(memnscan(fp_array, &fp_nonzero, 10, sizeof(fp_zero)), fp_array + 0);

	return -r;
}

TEST(callbacks_set){
	int r = 0;
	cb_t pre,
		 cbs,
		 post;


	memset(&pre, 0x0, sizeof(cb_t));
	memset(&cbs, 0x0, sizeof(cb_t));
	memset(&post, 0x0, sizeof(cb_t));

	r += TEST_INT_EQ(callbacks_set(&cbs, cb_t), false);

	cbs.foo = (void*)0x1;
	r += TEST_INT_EQ(callbacks_set(&cbs, cb_t), false);

	cbs.bar = (void*)0x1;
	r += TEST_INT_EQ(callbacks_set(&cbs, cb_t), true);

	return -r;
}

TEST(strerror){
	int r = 0;


	r += TEST_STR_EQ(strerror(0), "Success");
	r += TEST_STR_EQ(strerror(E_UNKNOWN), "Unknown");
	r += TEST_STR_EQ(strerror(0xfe), "Invalid errno");

	return -r;
}

TEST(itoa){
	int r = 0;
	char s[16];


	/* different base values */
	r += TEST_STR_EQ(itoa(10, 8, s, 15), "12");
	r += TEST_STR_EQ(itoa(10, 10, s, 15), "10");
	r += TEST_STR_EQ(itoa(10, 16, s, 15), "a");

	/* too large number */
	r += TEST_PTR_EQ(itoa(255, 10, s, 2), 0x0);

	return -r;
}

TEST(strtol){
	int r = 0;
	char *s;


	/* different base values */
	r += TEST_INT_EQ(strtol("10", 0x0, 10), 10);
	r += TEST_INT_EQ(strtol("10", 0x0, 8), 8);
	r += TEST_INT_EQ(strtol("deAdbEEF", 0x0, 16), 3735928559);

	/* auto detected base */
	r += TEST_INT_EQ(strtol("10", 0x0, 0), 10);
	r += TEST_INT_EQ(strtol("0x10", 0x0, 0), 16);
	r += TEST_INT_EQ(strtol("011", 0x0, 0), 9);

	/* signs */
	r += TEST_INT_EQ(strtol("0xf", 0x0, 16), 15);
	r += TEST_INT_EQ(strtol("+0xf", 0x0, 16), 15);
	r += TEST_INT_EQ(strtol("-0xf", 0x0, 16), -15);

	/* partial match */
	r += TEST_INT_EQ(strtol("10f", 0x0, 10), 10);
	r += TEST_INT_EQ(strtol("10f", 0x0, 16), 271);

	/* use endp */
	strtol("10f", &s, 10);
	r += TEST_STR_EQ(s, "f");

	strtol("0xfgo", &s, 0);
	r += TEST_STR_EQ(s, "go");

	/* invalid inputs */
	r += TEST_INT_EQ(strtol(" 0xfe", &s, 8), 0);
	r += TEST_INT_EQ(strtol("0xfe", &s, 8), 0);
	r += TEST_STR_EQ(s, "xfe");

	/* atoi */
	r += TEST_INT_EQ(atoi("10"), 10);
	r += TEST_INT_EQ(atoi("0xee"), 0);

	return -r;
}

TEST(strupr){
	int r = 0;
	char s[8];


	r += TEST_STR_EQ(strupr(""), "");
	r += TEST_STR_EQ(strupr("foo"), "FOO");
	r += TEST_STR_EQ(strupr("FOO"), "FOO");
	r += TEST_STR_EQ(strupr("123"), "123");
	r += TEST_STR_EQ(strupr("fOo123"), "FOO123");

	r += TEST_STR_EQ(strupr_r("foo", s, 2), "FO");

	return -r;
}

TEST(strcident){
	int r = 0;
	char s[8];


	r += TEST_STR_EQ(strcident(""), "");
	r += TEST_STR_EQ(strcident("foobar"), "foobar");
	r += TEST_STR_EQ(strcident("foo_bar"), "foo_bar");
	r += TEST_STR_EQ(strcident("foo-bar"), "foo_bar");

	r += TEST_STR_EQ(strcident_r("foo_bar", s, 5), "foo_b");

	return -r;
}

TEST(strdeesc){
	int r = 0;


	r += test_deesc("noesc", "noesc");
	r += test_deesc("\\a", "\a");
	r += test_deesc("\\\\", "\\");
	r += test_deesc("\\x", "\\x");
	r += test_deesc("ls \\\"foo\\\"", "ls \"foo\"");
	r += test_deesc("ls \\\\\"foo\\\\\"", "ls \\\"foo\\\"");

	return -r;
}

static int test_deesc(char const *s, char const *ref){
	size_t len = strlen(s);
	char line[len + 1];


	strcpy(line, s);
	strdeesc(line, len, esc_resolve);

	return TEST_STR_EQ(line, ref);
}

static int esc_resolve(int c){
	switch(c){
	case 'a':	return '\a';
	case '\\':	return '\\';
	case '"':	return '"';
	default:	return ESC_RESOLVE_NONE;
	}
}
