#ifndef TESTCASE_H
#define TESTCASE_H


/* brickos header */
#include <sys/compiler.h>

/* host header */
#include <unistd.h>


/* prototypes */
// unable to include stdio.h due to conflicts with brickos
// hence declare the required prototypes separately
int snprintf(char *str, size_t size, char const* format, ...);


/* macros */
#define test_case(hdlr, desc) \
	static test_case_t test_case_##hdlr __section(".test_case_hdlr") __used = hdlr; \
	static char const test_case_desc_str_##hdlr[]  __used = desc; \
	static char const* test_case_desc_##hdlr __section(".test_case_desc") __used = test_case_desc_str_##hdlr;

#define tlog(log, fmt, ...){ \
	char _s[256]; \
	int _n; \
	\
	\
	_n = snprintf(_s, 255, fmt, ##__VA_ARGS__); \
	if(_n) \
		write(log, _s, _n); \
}

#define check_int(log, expr, ref)({ \
	int _res; \
	\
	\
	_res = expr; \
	if(_res == ref) \
		tlog(log, "%s: res(%d) == ref(%d)\n", #expr, _res, ref) \
	else \
		tlog(log, "%s: res(%d) != ref(%d)\n", #expr, _res, ref) \
	\
	(_res == ref) ? 0 : 1; \
})

#define check_str(log, expr, s, ref)({ \
	expr; \
	if(strcmp(s, ref) == 0) \
		tlog(log, "%s: res('%s') == ref('%s')\n", #expr, s, ref) \
	else \
		tlog(log, "%s: res('%s') != ref('%s')\n", #expr, s, ref) \
	\
	(strcmp(s, ref) == 0) ? 0 : 1; \
})

#define check_strn(log, expr, s, ref, n)({ \
	expr; \
	if(strncmp(s, ref, n) == 0) \
		tlog(log, "%s: res('%s') == ref('%s')\n", #expr, s, ref) \
	else \
		tlog(log, "%s: res('%s') != ref('%s')\n", #expr, s, ref) \
	\
	(strncmp(s, ref, n) == 0) ? 0 : 1; \
})


/* types */
typedef int (*test_case_t)(int fd_log);


#endif // TESTCASE_H
