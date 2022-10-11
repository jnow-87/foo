/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* brickos header */
#include <config/config.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <test/test.h>

/* host header */
#include <unistd.h>


/* macros */
#define BUF_SIZE	512
#define DUMMY_SPEC	" %s"
#define DUMMY_VALUE	"bar"
#define DUMMY_STR	" bar"


/* local/static prototypes */
static int test(char const *ref, char const *s, ...);
static size_t fprintf(FILE *f, char const *format, ...);
static char putc(char c, struct FILE *stream);


/* local functions */
TEST(snprintf){
	int r = 0;
	char s[16];


	r += TEST_INT_EQ(snprintf(s, 2, "%d", 100), 1);
	r += TEST_STR_EQ(s, "1");

	return -r;
}

TEST(vfprintf){
	int r = 0;
	char tmp[3];
	long long int len;


	/* blank */
	r += test("foo",					"foo");

	/* *-width and *-precision */
	r += test("  010",					"%*.*d", 5, 3, 10);

	/* % */
	r += test("%",						"%");
	r += test("%",						"%%");

	/* n */
	r += test("123foo   10456",			"123%s%5d%hhn456", "foo", 10, &len);
	r += test("11",						"%hhd", (char)len);
	r += test("123foo   10456",			"123%s%5d%hn456", "foo", 10, &len);
	r += test("11",						"%hd", (short int)len);
	r += test("123foo   10456",			"123%s%5d%n456", "foo", 10, &len);
	r += test("11",						"%d", (int)len);
#ifdef CONFIG_PRINTF_LONG
	r += test("123foo   10456",			"123%s%5d%ln456", "foo", 10, &len);
	r += test("11",						"%ld", (long int)len);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	r += test("123foo   10456",			"123%s%5d%Ln456", "foo", 10, &len);
	r += test("11",						"%Ld", len);
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	r += test("123foo   10456",			"123%s%5d%zn456", "foo", 10, &len);
	r += test("11",						"%zd", (size_t)len);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	r += test("123foo   10456",			"123%s%5d%tn456", "foo", 10, &len);
	r += test("11",						"%td", (ptrdiff_t)len);
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	r += test("123foo   10456",			"123%s%5d%jn456", "foo", 10, &len);
	r += test("11",						"%jd", (intmax_t)len);
#endif // CONFIG_PRINTF_INTMAX

	// test is not allowed to write more than defined size to tmp (char in this case)
	memset(tmp, 0, 3);

	// this test is intended to fail, it only prepares tmp for the next one
	test("intended to fail",			"%280s%hhn", "foo", tmp + 1);
	r += test("0 24 0",					"%hhd %hhd %hhd", tmp[0], tmp[1], tmp[2]);

	/* c */
	r += test("d",						"%c", 'd');
	r += test("d    ",					"%-5c", 'd');
	r += test("    d",					"%5.7c", 'd');

	/* s */
	r += test("(null)",					"%s", 0x0);
	r += test("test",					"%s", "test");
	r += test("  tes",					"%5.3s", "test");
	r += test("tes  ",					"%-5.3s", "test");
	r += test("test ",					"%-5.4s", "test");

	/* i, d */
	r += test("0",						"%d", 0);
	r += test("",						"%.0d", 0);
	r += test("1",						"%.0d", 1);
	r += test("10",						"%d", 10);
	r += test("00010",					"%05d", 10);
	r += test("  010",					"%5.3d", 10);
	r += test("  010",					"%05.3d", 10);
	r += test("010  ",					"%-5.3d", 10);
	r += test(" +010",					"%+5.3d", 10);
	r += test("  010",					"% 5.3d", 10);
	r += test(" -010",					"%+5.3d", -10);

	r += test("10",						"%i", 10);
	r += test("  011",					"%5.3i", 11);
	r += test("  011",					"%05.3i", 11);
	r += test("011  ",					"%-5.3i", 11);
	r += test(" +011",					"%+5.3i", 11);
	r += test("  011",					"% 5.3i", 11);
	r += test(" -011",					"%+5.3i", -11);

	/* u */
	r += test("12",						"%u", 12);
	r += test("  012",					"%5.3u", 12);
	r += test("  012",					"%05.3u", 12);
	r += test("012  ",					"%-5.3u", 12);
	r += test("  012",					"%+5.3u", 12);
	r += test("  012",					"% 5.3u", 12);
	r += test("4294967284",				"%+5.3u", -12);

	/* x, X */
	r += test("c",						"%x", 12);
	r += test("  00c",					"%5.3x", 12);
	r += test("  00c",					"%05.3x", 12);
	r += test("00c  ",					"%-5.3x", 12);
	r += test("  00c",					"%+5.3x", 12);
	r += test("  00c",					"% 5.3x", 12);
	r += test("0x00c",					"%#5.3x", 12);
	r += test("fffffff4",				"%+5.3x", -12);

	r += test("D",						"%X", 13);
	r += test("  00D",					"%5.3X", 13);
	r += test("  00D",					"%05.3X", 13);
	r += test("00D  ",					"%-5.3X", 13);
	r += test("  00D",					"%+5.3X", 13);
	r += test("  00D",					"% 5.3X", 13);
	r += test("0X00D",					"%#5.3X", 13);
	r += test("FFFFFFF3",				"%+5.3X", -13);

	/* o */
	r += test("16",						"%o", 14);
	r += test("  016",					"%5.3o", 14);
	r += test("    0",					"%#5.1o", 0);
	r += test("016",					"%#.1o", 14);
	r += test("  016",					"%05.3o", 14);
	r += test("016  ",					"%-5.3o", 14);
	r += test("  016",					"%+5.3o", 14);
	r += test("  016",					"% 5.3o", 14);
	r += test("  016",					"%#5.3o", 14);
	r += test("37777777762",			"%+5.3o", -14);

	/* p */
	r += test("0xfe",					"%p", (void*)0xfe);
	r += test("0x0",					"%p", (void*)0x0);
	r += test("0x0",					"%#p",(void*) 0x0);
	r += test("0x0fe",					"%5.3p", (void*)0xfe);
	r += test("0x0fe",					"%05.3p", (void*)0xfe);
	r += test("0x0fe",					"%-5.3p", (void*)0xfe);
	r += test("+0x0fe",					"%+5.3p", (void*)0xfe);
	r += test(" 0x0fe",					"% 5.3p", (void*)0xfe);
	r += test("0x0fe",					"%#5.3p", (void*)0xfe);

#if CONFIG_REGISTER_WIDTH >= 32 \
 || defined(CONFIG_PRINTF_LONG) \
 || defined(CONFIG_PRINTF_SIZET)
	r += test("+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
#elif CONFIG_REGISTER_WIDTH == 8
	r += test("+0xffffff02",			"%+5.3p", (void*)-0xfe);
#else
	#warning "unhandled condition for pointer size"
#endif // CONFIG_REGISTER_WIDTH

	/* f, F, e, E, g, G, a, A */
	r += test("%f",						"%f");
	r += test("%F",						"%F");
	r += test("%e",						"%e");
	r += test("%E",						"%E");
	r += test("%g",						"%g");
	r += test("%G",						"%G");
	r += test("%a",						"%a");
	r += test("%A",						"%A");

	/* length */
	r += test("-10",					"%hhd", (char)-10);
	r += test("10",						"%hhd", (char)10);
	r += test("246",					"%hhu", (unsigned char)-10);
	r += test("10",						"%hhu", (unsigned char)10);

	r += test("-10",					"%hd", (short int)-10);
	r += test("10",						"%hd", (short int)10);
	r += test("65526",					"%hu", (unsigned short int)-10);
	r += test("10",						"%hu", (unsigned short int)10);

#ifdef CONFIG_PRINTF_LONG
	r += test("-10",					"%ld", (long int)-10);
	r += test("10",						"%ld", (long int)10);
	r += test("18446744073709551606",	"%lu", (unsigned long int)-10);
	r += test("10",						"%lu", (unsigned long int)10);
#else
	r += test("%ld",					"%ld", (long int)-10);
	r += test("%ld",					"%ld", (long int)10);
	r += test("%lu",					"%lu", (unsigned long int)-10);
	r += test("%lu",					"%lu", (unsigned long int)10);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	r += test("-10",					"%lld", (long long int)-10);
	r += test("10",						"%lld", (long long int)10);
	r += test("18446744073709551606",	"%llu", (unsigned long long int)-10);
	r += test("10",						"%llu", (unsigned long long int)10);
	r += test("-10",					"%Ld", (long long int)-10);
	r += test("10",						"%Ld", (long long int)10);
	r += test("18446744073709551606",	"%Lu", (unsigned long long int)-10);
	r += test("10",						"%Lu", (unsigned long long int)10);
	r += test("%Lf",					"%Lf");
#else
	r += test("%lld",					"%lld", (long long int)-10);
	r += test("%lld",					"%lld", (long long int)10);
	r += test("%llu",					"%llu", (unsigned long long int)-10);
	r += test("%llu",					"%llu", (unsigned long long int)10);
	r += test("%Lu",					"%Lu", (unsigned long long int)-10);
	r += test("%Ld",					"%Ld", (long long int)-10);
	r += test("%Ld",					"%Ld", (long long int)10);
	r += test("%Lu",					"%Lu", (unsigned long long int)10);
	r += test("%Lf",					"%Lf");
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_INTMAX
	r += test("-10",					"%jd", (intmax_t)-10);
	r += test("10",						"%jd", (intmax_t)10);
	r += test("18446744073709551606",	"%ju", (uintmax_t)-10);
	r += test("10",						"%ju", (intmax_t)10);
#else
	r += test("%jd",					"%jd", (intmax_t)-10);
	r += test("%jd",					"%jd", (intmax_t)10);
	r += test("%ju",					"%ju", (uintmax_t)-10);
	r += test("%ju",					"%ju", (intmax_t)10);
#endif // CONFIG_PRINTF_INTMAX

#ifdef CONFIG_PRINTF_SIZET
	r += test("-10",					"%zd", (size_t)-10);
	r += test("10",						"%zd", (size_t)10);
	r += test("18446744073709551606",	"%zu", (size_t)-10);
	r += test("10",						"%zu", (size_t)10);
#else
	r += test("%zd",					"%zd", (size_t)-10);
	r += test("%zd",					"%zd", (size_t)10);
	r += test("%zu",					"%zu", (size_t)-10);
	r += test("%zu",					"%zu", (size_t)10);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	r += test("-10",					"%td", (ptrdiff_t)-10);
	r += test("10",						"%td", (ptrdiff_t)10);
	r += test("18446744073709551606",	"%tu", (ptrdiff_t)-10);
	r += test("10",						"%tu", (ptrdiff_t)10);
#else
	r += test("%td",					"%td", (ptrdiff_t)-10);
	r += test("%td",					"%td", (ptrdiff_t)10);
	r += test("%tu",					"%tu", (ptrdiff_t)-10);
	r += test("%tu",					"%tu", (ptrdiff_t)10);
#endif // CONFIG_PRINTF_PTRDIFF

	return -r;
};

TEST(vfprintf_inval){
	int r = 0;
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, 0x0);


	r += test("12", "%#u", 12);

	r += TEST_INT_EQ(fprintf(0x0, ""), 0);
	r += TEST_INT_EQ(fprintf(&fp, ""), 0);
	r += TEST_INT_EQ(fprintf(&fp, " "), 0);

	fp.putc = putc;
	r += TEST_INT_EQ(fprintf(&fp, ""), 0);

	r += TEST_INT_EQ(fprintf(&fp, "%5s", "1"), 0);

	return -r;
}

static int test(char const *ref, char const *s, ...){
	int r = 0;
	char buf[BUF_SIZE];
	FILE f = FILE_INITIALISER(0x0, buf, BUF_SIZE, 0x0);
	size_t len;
	char ref_ext[strlen(ref) + strlen(DUMMY_STR) + 1];
	va_list lst;


	sprintf(ref_ext, "%s%s", ref, DUMMY_STR);
	memset(buf, 'a', BUF_SIZE);

	va_start(lst, s);
	len = vfprintf(&f, s, lst);
	len += fprintf(&f, DUMMY_SPEC, DUMMY_VALUE);
	va_end(lst);

	r += TEST_INT_EQ(len, strlen(ref_ext));
	r += TEST_STR_EQ(f.wbuf, ref_ext);

	return r;
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
