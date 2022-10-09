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
	int n = 0;
	char s[16];


	n += TEST_INT_EQ(snprintf(s, 2, "%d", 100), 1);
	n += TEST_STR_EQ(s, "1");

	return -n;
}

TEST(vfprintf){
	unsigned int n = 0;
	char tmp[3];
	long long int len;


	/* blank */
	n += test("foo",					"foo");

	/* *-width and *-precision */
	n += test("  010",					"%*.*d", 5, 3, 10);

	/* % */
	n += test("%",						"%");
	n += test("%",						"%%");

	/* n */
	n += test("123foo   10456",			"123%s%5d%hhn456", "foo", 10, &len);
	n += test("11",						"%hhd", (char)len);
	n += test("123foo   10456",			"123%s%5d%hn456", "foo", 10, &len);
	n += test("11",						"%hd", (short int)len);
	n += test("123foo   10456",			"123%s%5d%n456", "foo", 10, &len);
	n += test("11",						"%d", (int)len);
#ifdef CONFIG_PRINTF_LONG
	n += test("123foo   10456",			"123%s%5d%ln456", "foo", 10, &len);
	n += test("11",						"%ld", (long int)len);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	n += test("123foo   10456",			"123%s%5d%Ln456", "foo", 10, &len);
	n += test("11",						"%Ld", len);
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	n += test("123foo   10456",			"123%s%5d%zn456", "foo", 10, &len);
	n += test("11",						"%zd", (size_t)len);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	n += test("123foo   10456",			"123%s%5d%tn456", "foo", 10, &len);
	n += test("11",						"%td", (ptrdiff_t)len);
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	n += test("123foo   10456",			"123%s%5d%jn456", "foo", 10, &len);
	n += test("11",						"%jd", (intmax_t)len);
#endif // CONFIG_PRINTF_INTMAX

	// test is not allowed to write more than defined size to tmp (char in this case)
	memset(tmp, 0, 3);

	// this test is intended to fail, it only prepares tmp for the next one
	test("intended to fail",			"%280s%hhn", "foo", tmp + 1);
	n += test("0 24 0",					"%hhd %hhd %hhd", tmp[0], tmp[1], tmp[2]);

	/* c */
	n += test("d",						"%c", 'd');
	n += test("d    ",					"%-5c", 'd');
	n += test("    d",					"%5.7c", 'd');

	/* s */
	n += test("(null)",					"%s", 0x0);
	n += test("test",					"%s", "test");
	n += test("  tes",					"%5.3s", "test");
	n += test("tes  ",					"%-5.3s", "test");
	n += test("test ",					"%-5.4s", "test");

	/* i, d */
	n += test("0",						"%d", 0);
	n += test("",						"%.0d", 0);
	n += test("1",						"%.0d", 1);
	n += test("10",						"%d", 10);
	n += test("00010",					"%05d", 10);
	n += test("  010",					"%5.3d", 10);
	n += test("  010",					"%05.3d", 10);
	n += test("010  ",					"%-5.3d", 10);
	n += test(" +010",					"%+5.3d", 10);
	n += test("  010",					"% 5.3d", 10);
	n += test(" -010",					"%+5.3d", -10);

	n += test("10",						"%i", 10);
	n += test("  011",					"%5.3i", 11);
	n += test("  011",					"%05.3i", 11);
	n += test("011  ",					"%-5.3i", 11);
	n += test(" +011",					"%+5.3i", 11);
	n += test("  011",					"% 5.3i", 11);
	n += test(" -011",					"%+5.3i", -11);

	/* u */
	n += test("12",						"%u", 12);
	n += test("  012",					"%5.3u", 12);
	n += test("  012",					"%05.3u", 12);
	n += test("012  ",					"%-5.3u", 12);
	n += test("  012",					"%+5.3u", 12);
	n += test("  012",					"% 5.3u", 12);
	n += test("4294967284",				"%+5.3u", -12);

	/* x, X */
	n += test("c",						"%x", 12);
	n += test("  00c",					"%5.3x", 12);
	n += test("  00c",					"%05.3x", 12);
	n += test("00c  ",					"%-5.3x", 12);
	n += test("  00c",					"%+5.3x", 12);
	n += test("  00c",					"% 5.3x", 12);
	n += test("0x00c",					"%#5.3x", 12);
	n += test("fffffff4",				"%+5.3x", -12);

	n += test("D",						"%X", 13);
	n += test("  00D",					"%5.3X", 13);
	n += test("  00D",					"%05.3X", 13);
	n += test("00D  ",					"%-5.3X", 13);
	n += test("  00D",					"%+5.3X", 13);
	n += test("  00D",					"% 5.3X", 13);
	n += test("0X00D",					"%#5.3X", 13);
	n += test("FFFFFFF3",				"%+5.3X", -13);

	/* o */
	n += test("16",						"%o", 14);
	n += test("  016",					"%5.3o", 14);
	n += test("    0",					"%#5.1o", 0);
	n += test("016",					"%#.1o", 14);
	n += test("  016",					"%05.3o", 14);
	n += test("016  ",					"%-5.3o", 14);
	n += test("  016",					"%+5.3o", 14);
	n += test("  016",					"% 5.3o", 14);
	n += test("  016",					"%#5.3o", 14);
	n += test("37777777762",			"%+5.3o", -14);

	/* p */
	n += test("0xfe",					"%p", (void*)0xfe);
	n += test("0x0",					"%p", (void*)0x0);
	n += test("0x0",					"%#p",(void*) 0x0);
	n += test("0x0fe",					"%5.3p", (void*)0xfe);
	n += test("0x0fe",					"%05.3p", (void*)0xfe);
	n += test("0x0fe",					"%-5.3p", (void*)0xfe);
	n += test("+0x0fe",					"%+5.3p", (void*)0xfe);
	n += test(" 0x0fe",					"% 5.3p", (void*)0xfe);
	n += test("0x0fe",					"%#5.3p", (void*)0xfe);

#if CONFIG_REGISTER_WIDTH >= 32 \
 || defined(CONFIG_PRINTF_LONG) \
 || defined(CONFIG_PRINTF_SIZET)
	n += test("+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
#elif CONFIG_REGISTER_WIDTH == 8
	n += test("+0xffffff02",			"%+5.3p", (void*)-0xfe);
#else
	#warning "unhandled condition for pointer size"
#endif // CONFIG_REGISTER_WIDTH

	/* f, F, e, E, g, G, a, A */
	n += test("%f",						"%f");
	n += test("%F",						"%F");
	n += test("%e",						"%e");
	n += test("%E",						"%E");
	n += test("%g",						"%g");
	n += test("%G",						"%G");
	n += test("%a",						"%a");
	n += test("%A",						"%A");

	/* length */
	n += test("-10",					"%hhd", (char)-10);
	n += test("10",						"%hhd", (char)10);
	n += test("246",					"%hhu", (unsigned char)-10);
	n += test("10",						"%hhu", (unsigned char)10);

	n += test("-10",					"%hd", (short int)-10);
	n += test("10",						"%hd", (short int)10);
	n += test("65526",					"%hu", (unsigned short int)-10);
	n += test("10",						"%hu", (unsigned short int)10);

#ifdef CONFIG_PRINTF_LONG
	n += test("-10",					"%ld", (long int)-10);
	n += test("10",						"%ld", (long int)10);
	n += test("18446744073709551606",	"%lu", (unsigned long int)-10);
	n += test("10",						"%lu", (unsigned long int)10);
#else
	n += test("%ld",					"%ld", (long int)-10);
	n += test("%ld",					"%ld", (long int)10);
	n += test("%lu",					"%lu", (unsigned long int)-10);
	n += test("%lu",					"%lu", (unsigned long int)10);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	n += test("-10",					"%lld", (long long int)-10);
	n += test("10",						"%lld", (long long int)10);
	n += test("18446744073709551606",	"%llu", (unsigned long long int)-10);
	n += test("10",						"%llu", (unsigned long long int)10);
	n += test("-10",					"%Ld", (long long int)-10);
	n += test("10",						"%Ld", (long long int)10);
	n += test("18446744073709551606",	"%Lu", (unsigned long long int)-10);
	n += test("10",						"%Lu", (unsigned long long int)10);
	n += test("%Lf",					"%Lf");
#else
	n += test("%lld",					"%lld", (long long int)-10);
	n += test("%lld",					"%lld", (long long int)10);
	n += test("%llu",					"%llu", (unsigned long long int)-10);
	n += test("%llu",					"%llu", (unsigned long long int)10);
	n += test("%Lu",					"%Lu", (unsigned long long int)-10);
	n += test("%Ld",					"%Ld", (long long int)-10);
	n += test("%Ld",					"%Ld", (long long int)10);
	n += test("%Lu",					"%Lu", (unsigned long long int)10);
	n += test("%Lf",					"%Lf");
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_INTMAX
	n += test("-10",					"%jd", (intmax_t)-10);
	n += test("10",						"%jd", (intmax_t)10);
	n += test("18446744073709551606",	"%ju", (uintmax_t)-10);
	n += test("10",						"%ju", (intmax_t)10);
#else
	n += test("%jd",					"%jd", (intmax_t)-10);
	n += test("%jd",					"%jd", (intmax_t)10);
	n += test("%ju",					"%ju", (uintmax_t)-10);
	n += test("%ju",					"%ju", (intmax_t)10);
#endif // CONFIG_PRINTF_INTMAX

#ifdef CONFIG_PRINTF_SIZET
	n += test("-10",					"%zd", (size_t)-10);
	n += test("10",						"%zd", (size_t)10);
	n += test("18446744073709551606",	"%zu", (size_t)-10);
	n += test("10",						"%zu", (size_t)10);
#else
	n += test("%zd",					"%zd", (size_t)-10);
	n += test("%zd",					"%zd", (size_t)10);
	n += test("%zu",					"%zu", (size_t)-10);
	n += test("%zu",					"%zu", (size_t)10);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	n += test("-10",					"%td", (ptrdiff_t)-10);
	n += test("10",						"%td", (ptrdiff_t)10);
	n += test("18446744073709551606",	"%tu", (ptrdiff_t)-10);
	n += test("10",						"%tu", (ptrdiff_t)10);
#else
	n += test("%td",					"%td", (ptrdiff_t)-10);
	n += test("%td",					"%td", (ptrdiff_t)10);
	n += test("%tu",					"%tu", (ptrdiff_t)-10);
	n += test("%tu",					"%tu", (ptrdiff_t)10);
#endif // CONFIG_PRINTF_PTRDIFF

	return -n;
};

TEST(vfprintf_inval){
	int n = 0;
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, 0x0);


	n += test("12", "%#u", 12);

	n += TEST_INT_EQ(fprintf(0x0, ""), 0);
	n += TEST_INT_EQ(fprintf(&fp, ""), 0);
	n += TEST_INT_EQ(fprintf(&fp, " "), 0);

	fp.putc = putc;
	n += TEST_INT_EQ(fprintf(&fp, ""), 0);

	n += TEST_INT_EQ(fprintf(&fp, "%5s", "1"), 0);

	return -n;
}

static int test(char const *ref, char const *s, ...){
	int n = 0;
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

	n += TEST_INT_EQ(len, strlen(ref_ext));
	n += TEST_STR_EQ(f.wbuf, ref_ext);

	return n;
}

static size_t fprintf(FILE *f, char const *format, ...){
	size_t n;
	va_list lst;


	va_start(lst, format);
	n = vfprintf(f, format, lst);
	va_end(lst);

	return n;
}

static char putc(char c, struct FILE *stream){
	return 0;
}
