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
#include <testing/testcase.h>

/* host header */
#include <unistd.h>


/* macros */
#define BUF_SIZE	512
#define DUMMY_SPEC	" %s"
#define DUMMY_VALUE	"bar"
#define DUMMY_STR	" bar"


/* local/static prototypes */
static size_t fprintf(FILE *f, char const *format, ...);
static char putc(char c, struct FILE *stream);


/* local functions */
static int test(int log, char const *ref, char const *s, ...){
	int n;
	size_t len;
	char ref_ext[strlen(ref) + strlen(DUMMY_STR) + 1];
	char buf[BUF_SIZE];
	FILE f = FILE_INITIALISER(0x0, buf, BUF_SIZE, 0x0);
	va_list lst;


	sprintf(ref_ext, "%s%s", ref, DUMMY_STR);
	memset(buf, 'a', BUF_SIZE);

	va_start(lst, s);
	len = vfprintf(&f, s, lst);
	len += fprintf(&f, DUMMY_SPEC, DUMMY_VALUE);
	va_end(lst);

	n = 0;
	n += check_int(log, len, strlen(ref_ext));
	n += check_str(log, {}, f.wbuf, ref_ext);

	return n;
}

static int tc_snprintf(int log){
	int n;
	char s[16];


	n = 0;
	n += check_int(log, snprintf(s, 2, "%d", 100), 1);
	n += check_str(log, , s, "1");

	return -n;
}

test_case(tc_snprintf, "snprintf");

static int tc_vfprintf(int log){
	unsigned int n;
	char tmp[3];
	long long int len;


	n = 0;


	/* blank */
	n += test(log,		"foo",					"foo");

	/* *-width and *-precision */
	n += test(log,		"  010",				"%*.*d", 5, 3, 10);

	/* % */
	n += test(log,		"%",					"%");
	n += test(log,		"%",					"%%");

	/* n */
	n += test(log,		"123foo   10456",		"123%s%5d%hhn456", "foo", 10, &len);
	n += test(log,		"11",					"%hhd", (char)len);
	n += test(log,		"123foo   10456",		"123%s%5d%hn456", "foo", 10, &len);
	n += test(log,		"11",					"%hd", (short int)len);
	n += test(log,		"123foo   10456",		"123%s%5d%n456", "foo", 10, &len);
	n += test(log,		"11",					"%d", (int)len);
#ifdef CONFIG_PRINTF_LONG
	n += test(log,		"123foo   10456",		"123%s%5d%ln456", "foo", 10, &len);
	n += test(log,		"11",					"%ld", (long int)len);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	n += test(log,		"123foo   10456",		"123%s%5d%Ln456", "foo", 10, &len);
	n += test(log,		"11",					"%Ld", len);
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	n += test(log,		"123foo   10456",		"123%s%5d%zn456", "foo", 10, &len);
	n += test(log,		"11",					"%zd", (size_t)len);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	n += test(log,		"123foo   10456",		"123%s%5d%tn456", "foo", 10, &len);
	n += test(log,		"11",					"%td", (ptrdiff_t)len);
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	n += test(log,		"123foo   10456",		"123%s%5d%jn456", "foo", 10, &len);
	n += test(log,		"11",					"%jd", (intmax_t)len);
#endif // CONFIG_PRINTF_INTMAX

	// test is not allowed to write more than defined size to tmp (char in this case)
	memset(tmp, 0, 3);

	// this test is intended to fail, it only prepares tmp for the next one
	test(log,			"intended to fail",		"%280s%hhn", "foo", tmp + 1);
	n += test(log,		"0 24 0",				"%hhd %hhd %hhd", tmp[0], tmp[1], tmp[2]);

	/* c */
	n += test(log,		"d",					"%c", 'd');
	n += test(log,		"d    ",				"%-5c", 'd');
	n += test(log,		"    d",				"%5.7c", 'd');

	/* s */
	n += test(log,		"test",					"%s", "test");
	n += test(log,		"  tes",				"%5.3s", "test");
	n += test(log,		"tes  ",				"%-5.3s", "test");
	n += test(log,		"test ",				"%-5.4s", "test");

	/* i, d */
	n += test(log,		"0",					"%d", 0);
	n += test(log,		"",						"%.0d", 0);
	n += test(log,		"1",					"%.0d", 1);
	n += test(log,		"10",					"%d", 10);
	n += test(log,		"00010",				"%05d", 10);
	n += test(log,		"  010",				"%5.3d", 10);
	n += test(log,		"  010",				"%05.3d", 10);
	n += test(log,		"010  ",				"%-5.3d", 10);
	n += test(log,		" +010",				"%+5.3d", 10);
	n += test(log,		"  010",				"% 5.3d", 10);
	n += test(log,		" -010",				"%+5.3d", -10);

	n += test(log,		"10",					"%i", 10);
	n += test(log,		"  011",				"%5.3i", 11);
	n += test(log,		"  011",				"%05.3i", 11);
	n += test(log,		"011  ",				"%-5.3i", 11);
	n += test(log,		" +011",				"%+5.3i", 11);
	n += test(log,		"  011",				"% 5.3i", 11);
	n += test(log,		" -011",				"%+5.3i", -11);

	/* u */
	n += test(log,		"12",					"%u", 12);
	n += test(log,		"  012",				"%5.3u", 12);
	n += test(log,		"  012",				"%05.3u", 12);
	n += test(log,		"012  ",				"%-5.3u", 12);
	n += test(log,		"  012",				"%+5.3u", 12);
	n += test(log,		"  012",				"% 5.3u", 12);
	n += test(log,		"4294967284",			"%+5.3u", -12);

	/* x, X */
	n += test(log,		"c",					"%x", 12);
	n += test(log,		"  00c",				"%5.3x", 12);
	n += test(log,		"  00c",				"%05.3x", 12);
	n += test(log,		"00c  ",				"%-5.3x", 12);
	n += test(log,		"  00c",				"%+5.3x", 12);
	n += test(log,		"  00c",				"% 5.3x", 12);
	n += test(log,		"0x00c",				"%#5.3x", 12);
	n += test(log,		"fffffff4",				"%+5.3x", -12);

	n += test(log,		"D",					"%X", 13);
	n += test(log,		"  00D",				"%5.3X", 13);
	n += test(log,		"  00D",				"%05.3X", 13);
	n += test(log,		"00D  ",				"%-5.3X", 13);
	n += test(log,		"  00D",				"%+5.3X", 13);
	n += test(log,		"  00D",				"% 5.3X", 13);
	n += test(log,		"0X00D",				"%#5.3X", 13);
	n += test(log,		"FFFFFFF3",				"%+5.3X", -13);

	/* o */
	n += test(log,		"16",					"%o", 14);
	n += test(log,		"  016",				"%5.3o", 14);
	n += test(log,		"    0",				"%#5.1o", 0);
	n += test(log,		"016",					"%#.1o", 14);
	n += test(log,		"  016",				"%05.3o", 14);
	n += test(log,		"016  ",				"%-5.3o", 14);
	n += test(log,		"  016",				"%+5.3o", 14);
	n += test(log,		"  016",				"% 5.3o", 14);
	n += test(log,		"  016",				"%#5.3o", 14);
	n += test(log,		"37777777762",			"%+5.3o", -14);

	/* p */
	n += test(log,		"0xfe",					"%p", (void*)0xfe);
	n += test(log,		"0x0",					"%p", (void*)0x0);
	n += test(log,		"0x0",					"%#p",(void*) 0x0);
	n += test(log,		"0x0fe",				"%5.3p", (void*)0xfe);
	n += test(log,		"0x0fe",				"%05.3p", (void*)0xfe);
	n += test(log,		"0x0fe",				"%-5.3p", (void*)0xfe);
	n += test(log,		"+0x0fe",				"%+5.3p", (void*)0xfe);
	n += test(log,		" 0x0fe",				"% 5.3p", (void*)0xfe);
	n += test(log,		"0x0fe",				"%#5.3p", (void*)0xfe);

#if defined(CONFIG_PRINTF_LONG) || defined(CONFIG_PRINTF_SIZET)
	n += test(log,		"+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
#elif CONFIG_REGISTER_WIDTH == 8
	n += test(log,		"+0xffffff02",			"%+5.3p", (void*)-0xfe);
#else
	#warning "unhandled condition for pointer size"
#endif // CONFIG_REGISTER_WIDTH

	/* f, F, e, E, g, G, a, A */
	n += test(log,		"%f",					"%f");
	n += test(log,		"%F",					"%F");
	n += test(log,		"%e",					"%e");
	n += test(log,		"%E",					"%E");
	n += test(log,		"%g",					"%g");
	n += test(log,		"%G",					"%G");
	n += test(log,		"%a",					"%a");
	n += test(log,		"%A",					"%A");

	/* length */
	n += test(log,		"-10",					"%hhd", (char)-10);
	n += test(log,		"10",					"%hhd", (char)10);
	n += test(log,		"246",					"%hhu", (unsigned char)-10);
	n += test(log,		"10",					"%hhu", (unsigned char)10);

	n += test(log,		"-10",					"%hd", (short int)-10);
	n += test(log,		"10",					"%hd", (short int)10);
	n += test(log,		"65526",				"%hu", (unsigned short int)-10);
	n += test(log,		"10",					"%hu", (unsigned short int)10);

#ifdef CONFIG_PRINTF_LONG
	n += test(log,		"-10",					"%ld", (long int)-10);
	n += test(log,		"10",					"%ld", (long int)10);
	n += test(log,		"18446744073709551606",	"%lu", (unsigned long int)-10);
	n += test(log,		"10",					"%lu", (unsigned long int)10);
#else
	n += test(log,		"%ld",					"%ld", (long int)-10);
	n += test(log,		"%ld",					"%ld", (long int)10);
	n += test(log,		"%lu",					"%lu", (unsigned long int)-10);
	n += test(log,		"%lu",					"%lu", (unsigned long int)10);
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	n += test(log,		"-10",					"%lld", (long long int)-10);
	n += test(log,		"10",					"%lld", (long long int)10);
	n += test(log,		"18446744073709551606",	"%llu", (unsigned long long int)-10);
	n += test(log,		"10",					"%llu", (unsigned long long int)10);
	n += test(log,		"-10",					"%Ld", (long long int)-10);
	n += test(log,		"10",					"%Ld", (long long int)10);
	n += test(log,		"18446744073709551606",	"%Lu", (unsigned long long int)-10);
	n += test(log,		"10",					"%Lu", (unsigned long long int)10);
	n += test(log,		"%Lf",					"%Lf");
#else
	n += test(log,		"%lld",					"%lld", (long long int)-10);
	n += test(log,		"%lld",					"%lld", (long long int)10);
	n += test(log,		"%llu",					"%llu", (unsigned long long int)-10);
	n += test(log,		"%llu",					"%llu", (unsigned long long int)10);
	n += test(log,		"%Lu",					"%Lu", (unsigned long long int)-10);
	n += test(log,		"%Ld",					"%Ld", (long long int)-10);
	n += test(log,		"%Ld",					"%Ld", (long long int)10);
	n += test(log,		"%Lu",					"%Lu", (unsigned long long int)10);
	n += test(log,		"%Lf",					"%Lf");
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_INTMAX
	n += test(log,		"-10",					"%jd", (intmax_t)-10);
	n += test(log,		"10",					"%jd", (intmax_t)10);
	n += test(log,		"18446744073709551606",	"%ju", (uintmax_t)-10);
	n += test(log,		"10",					"%ju", (intmax_t)10);
#else
	n += test(log,		"%jd",					"%jd", (intmax_t)-10);
	n += test(log,		"%jd",					"%jd", (intmax_t)10);
	n += test(log,		"%ju",					"%ju", (uintmax_t)-10);
	n += test(log,		"%ju",					"%ju", (intmax_t)10);
#endif // CONFIG_PRINTF_INTMAX

#ifdef CONFIG_PRINTF_SIZET
	n += test(log,		"-10",					"%zd", (size_t)-10);
	n += test(log,		"10",					"%zd", (size_t)10);
	n += test(log,		"18446744073709551606",	"%zu", (size_t)-10);
	n += test(log,		"10",					"%zu", (size_t)10);
#else
	n += test(log,		"%zd",					"%zd", (size_t)-10);
	n += test(log,		"%zd",					"%zd", (size_t)10);
	n += test(log,		"%zu",					"%zu", (size_t)-10);
	n += test(log,		"%zu",					"%zu", (size_t)10);
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	n += test(log,		"-10",					"%td", (ptrdiff_t)-10);
	n += test(log,		"10",					"%td", (ptrdiff_t)10);
	n += test(log,		"18446744073709551606",	"%tu", (ptrdiff_t)-10);
	n += test(log,		"10",					"%tu", (ptrdiff_t)10);
#else
	n += test(log,		"%td",					"%td", (ptrdiff_t)-10);
	n += test(log,		"%td",					"%td", (ptrdiff_t)10);
	n += test(log,		"%tu",					"%tu", (ptrdiff_t)-10);
	n += test(log,		"%tu",					"%tu", (ptrdiff_t)10);
#endif // CONFIG_PRINTF_PTRDIFF

	return -n;
};

test_case(tc_vfprintf, "vfprintf");

static int tc_vfprintf_inval(int log){
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, 0x0);
	int n;


	n = 0;

	n += test(log, "12", "%#u", 12);

	n += check_int(log, fprintf(0x0, ""), 0);
	n += check_int(log, fprintf(&fp, ""), 0);
	n += check_int(log, fprintf(&fp, " "), 0);

	fp.putc = putc;
	n += check_int(log, fprintf(&fp, ""), 0);

	n += check_int(log, fprintf(&fp, "%5s", "1"), 0);


	return -n;
}

test_case(tc_vfprintf_inval, "vfprintf-inval");

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
