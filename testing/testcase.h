#ifndef TESTCASE_H
#define TESTCASE_H


/* brickos header */
#include <sys/compiler.h>

/* host header */
#include <unistd.h>


/* prototypes */
// unable to include stdio.h due to conflicts with brickos
// hence declare the required prototypes separately
int dprintf(int fd, char const *format, ...);


/* macros */
#define test_case(_hdlr, _desc) \
	static char const test_case_desc_##_hdlr[]  __used = _desc; \
	static test_case_t test_case_##_hdlr __section(".test_cases") __used = { .hdlr = _hdlr, .desc = test_case_desc_##_hdlr, }

#define tlog(log, fmt, ...)	dprintf(log, fmt, ##__VA_ARGS__);

#define check_int(log, expr, ref)({ \
	int _res; \
	\
	\
	_res = expr; \
	if(_res == (ref)) \
		tlog(log, "%s: res(%d) == ref(%d)\n", #expr, _res, ref) \
	else \
		tlog(log, "%s: res(%d) != ref(%d)\n", #expr, _res, ref) \
	\
	(_res == (ref)) ? 0 : 1; \
})

#define check_ptr(log, expr, ref)({ \
	void *_res; \
	\
	\
	_res = expr; \
	if(_res == (ref)) \
		tlog(log, "%s: res(%#x) == ref(%#x)\n", #expr, _res, ref) \
	else \
		tlog(log, "%s: res(%#x) != ref(%#x)\n", #expr, _res, ref) \
	\
	(_res == (ref)) ? 0 : 1; \
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
typedef int (*tc_hdlr_t)(int fd_log);
typedef struct{
	tc_hdlr_t hdlr;
	char const *desc;
} test_case_t;


#endif // TESTCASE_H
