/* brickos header */
#include <config/config.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <testing/testcase.h>

/* host header */
#include <unistd.h>


/* macros */
#define BUF_SIZE	64
#define DUMMY_SPEC	" %s"
#define DUMMY_VALUE	"bar"
#define DUMMY_STR	" bar"

#define test(log, ref, s, ...)({ \
	char buf[BUF_SIZE]; \
	FILE f = FILE_INITIALISER(0x0, buf, BUF_SIZE, 0x0); \
	size_t len; \
	char const ref_ext[] = ref DUMMY_STR; \
	\
	\
	memset(buf, 'a', BUF_SIZE); \
	len = fprintf(&f, s DUMMY_SPEC, ##__VA_ARGS__, DUMMY_VALUE); \
	\
	if(len != strlen(ref_ext)) \
		tlog(log, "%s: length differ res(%d) != ref(%d)\n", s, len, strlen(ref_ext)) \
	\
	((char*)f.wbuf)[len] = 0; \
	if(strcmp(ref_ext, f.wbuf) == 0) \
		tlog(log, "%s: res('%s') == ref('%s')\n", s, f.wbuf, ref_ext) \
	\
	else \
		tlog(log, "%s: res('%s') != ref('%s')\n", s, f.wbuf, ref_ext) \
	\
	(len == strlen(ref_ext) && strcmp(ref_ext, f.wbuf) == 0) ? 0 : 1; \
})


/* local/static prototypes */
static size_t fprintf(FILE *f, char const *format, ...);


/* local functions */
static int tc_vfprintf(int log){
	unsigned int n;
	int tmp;


	n = 0;


	/* blank */
	n += test(log,		"foo",					"foo");

	/* *-width and *-precision */
	n += test(log,		"  010",				"%*.*d", 5, 3, 10);

	/* % */
	n += test(log,		"%",					"%%");

	/* n */
	n += test(log,		"123foo   10456",		"123%s%5d%n456", "foo", 10, &tmp);
	n += test(log,		"11",					"%d", tmp);

	/* c */
	n += test(log,		"d",					"%c", 'd');
	n += test(log,		"d    ",				"%-5c", 'd');
	n += test(log,		"    d",				"%5.7c", 'd');

	/* s */
	n += test(log,		"test",					"%s", "test");
	n += test(log,		"  tes",				"%5.3s", "test");
	n += test(log,		"tes  ",				"%-5.3s", "test");

	/* i, d */
	n += test(log,		"0",					"%d", 0);
	n += test(log,		"",						"%.0d", 0);
	n += test(log,		"10",					"%d", 10);
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

#if defined(CONFIG_PRINTF_LONG)
	n += test(log,		"+0xffffffffffffff02",	"%+5.3p", (void*)-0xfe);
#elif CONFIG_REGISTER_WIDTH == 8
	n += test(log,		"+0xffffff02",			"%+5.3p", (void*)-0xfe);
#else
	#warning "unhandled condition for pointer size"
#endif // CONFIG_REGISTER_WIDTH

	/* f, F, e, E, g, G, a, A */

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
#else
	n += test(log,		"%lld",					"%lld", (long long int)-10);
	n += test(log,		"%lld",					"%lld", (long long int)10);
	n += test(log,		"%llu",					"%llu", (unsigned long long int)-10);
	n += test(log,		"%llu",					"%llu", (unsigned long long int)10);
	n += test(log,		"%Lu",					"%Lu", (unsigned long long int)-10);
	n += test(log,		"%Ld",					"%Ld", (long long int)-10);
	n += test(log,		"%Ld",					"%Ld", (long long int)10);
	n += test(log,		"%Lu",					"%Lu", (unsigned long long int)10);
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


static size_t fprintf(FILE *f, char const *format, ...){
	size_t n;
	va_list lst;


	va_start(lst, format);
	n = vfprintf(f, format, lst);
	va_end(lst);

	return n;
}
