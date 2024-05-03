/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* brickos header */
#include <config/config.h>
#include <sys/devicetree.h>
#include <sys/limits.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <stdlib.h>
#include <test/test.h>

/* host header */
#include <unistd.h>


/* macros */
#define BUF_SIZE	512
#define DUMMY_SPEC	" %s"
#define DUMMY_VALUE	"bar"
#define DUMMY_STR	" bar"

#define TEST_VFPRINTF(ref, s, ...)	test(__LINE__, ref, s, ##__VA_ARGS__)


/* local/static prototypes */
static int test(size_t line, char const *ref, char const *s, ...);

static size_t fprintf(FILE *f, char const *format, ...);
static char putc(char c, struct FILE *stream);


/* static variables */
static char *vfprintf_buf = 0x0;


/* local functions */
TEST(snprintf){
	int r = 0;
	char s[16];


	r |= TEST_INT_EQ(snprintf(s, 2, "%d", 100), 1);
	r |= TEST_STR_EQ(s, "1");

	return -r;
}

TEST(vfprintf){
	int r = 0;
	char tmp[3];
	long long int len;


	ASSERT_PTR_NEQ(vfprintf_buf = malloc(BUF_SIZE), 0x0);

	/* blank */
	r |= TEST_VFPRINTF("foo",					"foo");

	/* *-width and *-precision */
	r |= TEST_VFPRINTF("  010",					"%*.*d", 5, 3, 10);

	/* % */
	r |= TEST_VFPRINTF("%",						"%");
	r |= TEST_VFPRINTF("%",						"%%");

	/* n */
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%hhn456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%hhd", (char)len);
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%hn456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%hd", (short int)len);
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%n456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%d", (int)len);
#ifdef CONFIG_PRINTF_LONG
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%ln456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%ld", (long int)len);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%Ln456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%Ld", len);
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%zn456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%zd", (size_t)len);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%tn456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%td", (ptrdiff_t)len);
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	r |= TEST_VFPRINTF("123foo   10456",		"123%s%5d%jn456", "foo", 10, &len);
	r |= TEST_VFPRINTF("11",					"%jd", (intmax_t)len);
#endif // CONFIG_PRINTF_INTMAX

	// test is not allowed to write more than defined size to tmp (char in this case)
	memset(tmp, 0, 3);

	// this test is intended to fail, it only prepares tmp for the next one
	TEST_VFPRINTF("intended to fail",			"%280s%hhn", "foo", tmp + 1);
	r |= TEST_VFPRINTF("0 24 0",				"%hhd %hhd %hhd", tmp[0], tmp[1], tmp[2]);

	/* c */
	r |= TEST_VFPRINTF("d",						"%c", 'd');
	r |= TEST_VFPRINTF("d    ",					"%-5c", 'd');
	r |= TEST_VFPRINTF("    d",					"%5.7c", 'd');

	/* s */
	r |= TEST_VFPRINTF("(null)",				"%s", 0x0);
	r |= TEST_VFPRINTF("test",					"%s", "test");
	r |= TEST_VFPRINTF("  tes",					"%5.3s", "test");
	r |= TEST_VFPRINTF("tes  ",					"%-5.3s", "test");
	r |= TEST_VFPRINTF("test ",					"%-5.4s", "test");

	/* i, d */
	r |= TEST_VFPRINTF("0",						"%d", 0);
	r |= TEST_VFPRINTF("",						"%.0d", 0);
	r |= TEST_VFPRINTF("1",						"%.0d", 1);
	r |= TEST_VFPRINTF("10",					"%d", 10);
	r |= TEST_VFPRINTF("00010",					"%05d", 10);
	r |= TEST_VFPRINTF("  010",					"%5.3d", 10);
	r |= TEST_VFPRINTF("  010",					"%05.3d", 10);
	r |= TEST_VFPRINTF("010  ",					"%-5.3d", 10);
	r |= TEST_VFPRINTF(" +010",					"%+5.3d", 10);
	r |= TEST_VFPRINTF("  010",					"% 5.3d", 10);
	r |= TEST_VFPRINTF(" -010",					"%+5.3d", -10);

	r |= TEST_VFPRINTF("10",					"%i", 10);
	r |= TEST_VFPRINTF("  011",					"%5.3i", 11);
	r |= TEST_VFPRINTF("  011",					"%05.3i", 11);
	r |= TEST_VFPRINTF("011  ",					"%-5.3i", 11);
	r |= TEST_VFPRINTF(" +011",					"%+5.3i", 11);
	r |= TEST_VFPRINTF("  011",					"% 5.3i", 11);
	r |= TEST_VFPRINTF(" -011",					"%+5.3i", -11);

	/* u */
	r |= TEST_VFPRINTF("12",					"%u", 12);
	r |= TEST_VFPRINTF("  012",					"%5.3u", 12);
	r |= TEST_VFPRINTF("  012",					"%05.3u", 12);
	r |= TEST_VFPRINTF("012  ",					"%-5.3u", 12);
	r |= TEST_VFPRINTF("  012",					"%+5.3u", 12);
	r |= TEST_VFPRINTF("  012",					"% 5.3u", 12);

#if UINT_MAX == 65535U
	r |= TEST_VFPRINTF("65524",					"%+5.3u", -12);
#else
	r |= TEST_VFPRINTF("4294967284",			"%+5.3u", -12);
#endif // UINT_MAX

	/* x, X */
	r |= TEST_VFPRINTF("c",						"%x", 12);
	r |= TEST_VFPRINTF("  00c",					"%5.3x", 12);
	r |= TEST_VFPRINTF("  00c",					"%05.3x", 12);
	r |= TEST_VFPRINTF("00c  ",					"%-5.3x", 12);
	r |= TEST_VFPRINTF("  00c",					"%+5.3x", 12);
	r |= TEST_VFPRINTF("  00c",					"% 5.3x", 12);
	r |= TEST_VFPRINTF("0x00c",					"%#5.3x", 12);

#if UINT_MAX == 65535U
	r |= TEST_VFPRINTF(" fff4",					"%+5.3x", -12);
#else
	r |= TEST_VFPRINTF("fffffff4",				"%+5.3x", -12);
#endif // UINT_MAX

	r |= TEST_VFPRINTF("D",						"%X", 13);
	r |= TEST_VFPRINTF("  00D",					"%5.3X", 13);
	r |= TEST_VFPRINTF("  00D",					"%05.3X", 13);
	r |= TEST_VFPRINTF("00D  ",					"%-5.3X", 13);
	r |= TEST_VFPRINTF("  00D",					"%+5.3X", 13);
	r |= TEST_VFPRINTF("  00D",					"% 5.3X", 13);
	r |= TEST_VFPRINTF("0X00D",					"%#5.3X", 13);

#if UINT_MAX == 65535U
	r |= TEST_VFPRINTF(" FFF3",					"%+5.3X", -13);
#else
	r |= TEST_VFPRINTF("FFFFFFF3",				"%+5.3X", -13);
#endif // UINT_MAX

	/* o */
	r |= TEST_VFPRINTF("16",					"%o", 14);
	r |= TEST_VFPRINTF("  016",					"%5.3o", 14);
	r |= TEST_VFPRINTF("    0",					"%#5.1o", 0);
	r |= TEST_VFPRINTF("016",					"%#.1o", 14);
	r |= TEST_VFPRINTF("  016",					"%05.3o", 14);
	r |= TEST_VFPRINTF("016  ",					"%-5.3o", 14);
	r |= TEST_VFPRINTF("  016",					"%+5.3o", 14);
	r |= TEST_VFPRINTF("  016",					"% 5.3o", 14);
	r |= TEST_VFPRINTF("  016",					"%#5.3o", 14);

#if UINT_MAX == 65535U
	r |= TEST_VFPRINTF("177762",				"%+5.3o", -14);
#else
	r |= TEST_VFPRINTF("37777777762",			"%+5.3o", -14);
#endif // UINT_MAX

	/* p */
	r |= TEST_VFPRINTF("0xfe",					"%p", (void*)0xfe);
	r |= TEST_VFPRINTF("0x0",					"%p", (void*)0x0);
	r |= TEST_VFPRINTF("0x0",					"%#p",(void*) 0x0);
	r |= TEST_VFPRINTF("0x0fe",					"%5.3p", (void*)0xfe);
	r |= TEST_VFPRINTF("0x0fe",					"%05.3p", (void*)0xfe);
	r |= TEST_VFPRINTF("0x0fe",					"%-5.3p", (void*)0xfe);
	r |= TEST_VFPRINTF("+0x0fe",				"%+5.3p", (void*)0xfe);
	r |= TEST_VFPRINTF(" 0x0fe",				"% 5.3p", (void*)0xfe);
	r |= TEST_VFPRINTF("0x0fe",					"%#5.3p", (void*)0xfe);

#if defined(CONFIG_PRINTF_LONG) || defined(CONFIG_PRINTF_SIZET)
# ifndef BUILD_HOST
#  if DEVTREE_ARCH_ADDR_WIDTH == 16
	r |= TEST_VFPRINTF("+0xff02",				"%+5.3p", (void*)-0xfe);
#  elif DEVTREE_ARCH_ADDR_WIDTH == 32
	r |= TEST_VFPRINTF("+0xffffff02",			"%+5.3p", (void*)-0xfe);
#  elif DEVTREE_ARCH_ADDR_WIDTH == 64
	r |= TEST_VFPRINTF("+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
#  else
	STATIC_ASSERT(!"untested size");
#  endif // DEVTREE_ARCH_ADDR_WIDTH
# else // BUILD_HOST
	r |= TEST_VFPRINTF("+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
# endif // BUILD_HOST
#endif // CONFIG_PRINTF_LONG || CONFIG_PRINTF_SIZET

	/* f, F, e, E, g, G, a, A */
	r |= TEST_VFPRINTF("%f",					"%f");
	r |= TEST_VFPRINTF("%F",					"%F");
	r |= TEST_VFPRINTF("%e",					"%e");
	r |= TEST_VFPRINTF("%E",					"%E");
	r |= TEST_VFPRINTF("%g",					"%g");
	r |= TEST_VFPRINTF("%G",					"%G");
	r |= TEST_VFPRINTF("%a",					"%a");
	r |= TEST_VFPRINTF("%A",					"%A");

	/* length */
	r |= TEST_VFPRINTF("-10",					"%hhd", (char)-10);
	r |= TEST_VFPRINTF("10",					"%hhd", (char)10);
	r |= TEST_VFPRINTF("246",					"%hhu", (unsigned char)-10);
	r |= TEST_VFPRINTF("10",					"%hhu", (unsigned char)10);

	r |= TEST_VFPRINTF("-10",					"%hd", (short int)-10);
	r |= TEST_VFPRINTF("10",					"%hd", (short int)10);
	r |= TEST_VFPRINTF("65526",					"%hu", (unsigned short int)-10);
	r |= TEST_VFPRINTF("10",					"%hu", (unsigned short int)10);

#ifdef CONFIG_PRINTF_LONG
	r |= TEST_VFPRINTF("-10",					"%ld", (long int)-10);
	r |= TEST_VFPRINTF("10",					"%ld", (long int)10);
	r |= TEST_VFPRINTF("10",					"%lu", (unsigned long int)10);

# if ULONG_MAX == 4294967295U
	r |= TEST_VFPRINTF("4294967286",			"%lu", (unsigned long int)-10);
# elif ULONG_MAX == 18446744073709551615U
	r |= TEST_VFPRINTF("18446744073709551606",	"%lu", (unsigned long int)-10);
# else
	STATIC_ASSERT(!"untested size");
# endif // ULONG_MAX
#else
	r |= TEST_VFPRINTF("%ld",					"%ld", (long int)-10);
	r |= TEST_VFPRINTF("%ld",					"%ld", (long int)10);
	r |= TEST_VFPRINTF("%lu",					"%lu", (unsigned long int)-10);
	r |= TEST_VFPRINTF("%lu",					"%lu", (unsigned long int)10);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	r |= TEST_VFPRINTF("-10",					"%lld", (long long int)-10);
	r |= TEST_VFPRINTF("10",					"%lld", (long long int)10);
	r |= TEST_VFPRINTF("18446744073709551606",	"%llu", (unsigned long long int)-10);
	r |= TEST_VFPRINTF("10",					"%llu", (unsigned long long int)10);
	r |= TEST_VFPRINTF("-10",					"%Ld", (long long int)-10);
	r |= TEST_VFPRINTF("10",					"%Ld", (long long int)10);
	r |= TEST_VFPRINTF("18446744073709551606",	"%Lu", (unsigned long long int)-10);
	r |= TEST_VFPRINTF("10",					"%Lu", (unsigned long long int)10);
	r |= TEST_VFPRINTF("%Lf",					"%Lf");
#else
	r |= TEST_VFPRINTF("%lld",					"%lld", (long long int)-10);
	r |= TEST_VFPRINTF("%lld",					"%lld", (long long int)10);
	r |= TEST_VFPRINTF("%llu",					"%llu", (unsigned long long int)-10);
	r |= TEST_VFPRINTF("%llu",					"%llu", (unsigned long long int)10);
	r |= TEST_VFPRINTF("%Lu",					"%Lu", (unsigned long long int)-10);
	r |= TEST_VFPRINTF("%Ld",					"%Ld", (long long int)-10);
	r |= TEST_VFPRINTF("%Ld",					"%Ld", (long long int)10);
	r |= TEST_VFPRINTF("%Lu",					"%Lu", (unsigned long long int)10);
	r |= TEST_VFPRINTF("%Lf",					"%Lf");
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_INTMAX
	r |= TEST_VFPRINTF("-10",					"%jd", (intmax_t)-10);
	r |= TEST_VFPRINTF("10",					"%jd", (intmax_t)10);
	r |= TEST_VFPRINTF("18446744073709551606",	"%ju", (uintmax_t)-10);
	r |= TEST_VFPRINTF("10",					"%ju", (intmax_t)10);
#else
	r |= TEST_VFPRINTF("%jd",					"%jd", (intmax_t)-10);
	r |= TEST_VFPRINTF("%jd",					"%jd", (intmax_t)10);
	r |= TEST_VFPRINTF("%ju",					"%ju", (uintmax_t)-10);
	r |= TEST_VFPRINTF("%ju",					"%ju", (intmax_t)10);
#endif // CONFIG_PRINTF_INTMAX

#ifdef CONFIG_PRINTF_SIZET
	r |= TEST_VFPRINTF("-10",					"%zd", (size_t)-10);
	r |= TEST_VFPRINTF("10",					"%zd", (size_t)10);
	r |= TEST_VFPRINTF("10",					"%zu", (size_t)10);

# if SIZE_MAX == 65535U
	r |= TEST_VFPRINTF("65526",					"%zu", (size_t)-10);
# elif SIZE_MAX == 4294967295U
	r |= TEST_VFPRINTF("4294967286",			"%zu", (size_t)-10);
# elif SIZE_MAX == 18446744073709551615U
	r |= TEST_VFPRINTF("18446744073709551606",	"%zu", (size_t)-10);
# else
	STATIC_ASSERT(!"untested size");
# endif // SIZE_MAX
#else
	r |= TEST_VFPRINTF("%zd",					"%zd", (size_t)-10);
	r |= TEST_VFPRINTF("%zd",					"%zd", (size_t)10);
	r |= TEST_VFPRINTF("%zu",					"%zu", (size_t)-10);
	r |= TEST_VFPRINTF("%zu",					"%zu", (size_t)10);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	r |= TEST_VFPRINTF("-10",					"%td", (ptrdiff_t)-10);
	r |= TEST_VFPRINTF("10",					"%td", (ptrdiff_t)10);
	r |= TEST_VFPRINTF("10",					"%tu", (ptrdiff_t)10);
# if PTRDIFF_MAX == 2147483647
	r |= TEST_VFPRINTF("4294967286",			"%tu", (ptrdiff_t)-10);
# elif PTRDIFF_MAX == 9223372036854775807
	r |= TEST_VFPRINTF("18446744073709551606",	"%tu", (ptrdiff_t)-10);
# else
	STATIC_ASSERT(!"untested size");
# endif // PTRDIFF_MAX
#else
	r |= TEST_VFPRINTF("%td",					"%td", (ptrdiff_t)-10);
	r |= TEST_VFPRINTF("%td",					"%td", (ptrdiff_t)10);
	r |= TEST_VFPRINTF("%tu",					"%tu", (ptrdiff_t)-10);
	r |= TEST_VFPRINTF("%tu",					"%tu", (ptrdiff_t)10);
#endif // CONFIG_PRINTF_PTRDIFF

	free(vfprintf_buf);

	return -r;
};

TEST(vfprintf_inval){
	int r = 0;
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, 0x0);


	ASSERT_PTR_NEQ(vfprintf_buf = malloc(BUF_SIZE), 0x0);

	r |= TEST_VFPRINTF("12", "%#u", 12);

	r |= TEST_INT_EQ(fprintf(0x0, ""), 0);
	r |= TEST_INT_EQ(fprintf(&fp, ""), 0);
	r |= TEST_INT_EQ(fprintf(&fp, " "), 0);

	fp.putc = putc;
	r |= TEST_INT_EQ(fprintf(&fp, ""), 0);

	r |= TEST_INT_EQ(fprintf(&fp, "%5s", "1"), 0);

	free(vfprintf_buf);

	return -r;
}

static int test(size_t line, char const *ref, char const *s, ...){
	int r = 0;
	FILE f = FILE_INITIALISER(0x0, vfprintf_buf, BUF_SIZE, 0x0);
	size_t len;
	char ref_ext[strlen(ref) + strlen(DUMMY_STR) + 1];
	va_list lst;


	sprintf(ref_ext, "%s%s", ref, DUMMY_STR);
	memset(vfprintf_buf, 'a', BUF_SIZE);

	va_start(lst, s);
	len = vfprintf(&f, s, lst);
	len += fprintf(&f, DUMMY_SPEC, DUMMY_VALUE);
	va_end(lst);

	TEST_LOG("test: line=%zu, s=%s\n", line, s);
	r |= TEST_INT_EQ(len, strlen(ref_ext));
	r |= TEST_STR_EQ(f.wbuf, ref_ext);

	return -r;
}

static size_t fprintf(FILE *f, char const *format, ...){
	size_t r;
	va_list lst;


	va_start(lst, format);
	r = vfprintf(f, format, lst);
	va_end(lst);

	return r;
}

static char putc(char c, struct FILE *stream){
	return 0;
}
