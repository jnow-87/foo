/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/memory.h>
#include <sys/string.h>
#include <sys/patmat.h>
#include <testing/testcase.h>
#include <testing/memory.h>


/* types */
typedef struct{
	patmat_spec_type_t type;
	union{
		char c;
		int i;
		char *s;
	} result;
} expect_t;


/* local functions */
static int test(int log, patmat_t *pm, char const *input, ssize_t retval, size_t nexpect, expect_t *expect){
	int n;
	size_t i;
	void *results[nexpect];
	ssize_t index;


	if(pm == 0x0)
		return -1;

	index = retval;

	if(retval < 0)
		index = -1;

	patmat_reset(pm);

	tlog(log, "match \"%s\"\n", input);

	n = 0;
	n += check_int(log, patmat_match_string(pm, input), retval);
	n += check_int(log, patmat_get_results(pm, results), index);

	for(i=0; index>=0 && i<nexpect; i++){
		switch(expect[i].type){
		case PMS_INT:	n += check_int(log, PATMAT_RESULT_INT(results, i), expect[i].result.i); break;
		case PMS_CHAR:	n += check_int(log, PATMAT_RESULT_CHAR(results, i), expect[i].result.c); break;
		case PMS_STR:	n += check_str(log, , PATMAT_RESULT_STR(results, i), expect[i].result.s); break;
		default:		n += 1; break;
		}
	}

	return -n;
}

static int tc_patmat(int log){
	unsigned int n;
	char const *patterns[] = {
		"%d ",
		"%c%d ",
		"%df00",
		"f00%d ",
		"%df0%d ",
		"foo%dbbar%d ",
		"foo%10sbar",
		"%cfoo%dbar",
		"foo%10sbat%cfoo%d ",
		"foo%c ",
		"bar%10sbar%d ",
		"coverage",
	};
	patmat_t *pm;


	n = 0;

	pm = patmat_init(patterns, sizeof_array(patterns));

	/* special cases */
	n += check_int(log, test(log, pm, "19 ", 0, 1, (expect_t[]){{ .type = -1, .result.i = 19 }}), -1);
	n += test(log, pm, "1+", PM_NOMATCH, 0, 0x0);
	n += test(log, pm, "", PM_NOMATCH, 0, 0x0);

	/* no match */
	n += test(log, pm, "foo", PM_NOMATCH, 0, 0x0);

	/* match */
	n += test(log, pm, "19 ",		0, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 19 }});
	n += test(log, pm, "13f00",		2, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 13 }});
	n += test(log, pm, "f0011 ",	3, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 11 }});
	n += test(log, pm, "foofoobar",	6, 1, (expect_t[]){{ .type = PMS_STR, .result.s = "foo" }});
	n += test(log, pm, "fooxbar",	6, 1, (expect_t[]){{ .type = PMS_STR, .result.s = "x" }});
	n += test(log, pm, "foot ",		9, 1, (expect_t[]){{ .type = PMS_CHAR, .result.c = 't' }});

	n += test(log, pm, "17f0111 ", 4, 2, (expect_t[]){
		{ .type = PMS_INT, .result.i = 17 },
		{ .type = PMS_INT, .result.i = 111 },
	});

	n += test(log, pm, "foo13bbar19 ", 5, 2, (expect_t[]){
		{ .type = PMS_INT, .result.i = 13 },
		{ .type = PMS_INT, .result.i = 19 },
	});

	n += test(log, pm, "xfoo13bar", 7, 2, (expect_t[]){
		{ .type = PMS_CHAR, .result.c = 'x' },
		{ .type = PMS_INT, .result.i = 13 },
	});

	n += test(log, pm, "footestbatefoo17 ", 8, 3, (expect_t[]){
		{ .type = PMS_STR, .result.s = "test" },
		{ .type = PMS_CHAR, .result.c = 'e' },
		{ .type = PMS_INT, .result.i = 17 },
	});

	/* open issues */
	// TODO - cf. issue 196
	// 	The following test matches even though it contains a specifier
//	n += test(log, pm, "%d ", PM_NOMATCH, 0 , 0x0);

	// TODO - cf. issue 193
	// 	The following test doesn't match a pattern, since patmat is not able to match two
	// 	consecutive specifiers.
	n += test(log, pm, "f13 ", PM_NOMATCH, 0, 0x0);
//		(expect_t[]){
//			{ .type = PMS_CHAR, .result.c = 'f' },
//			{ .type = PMS_INT, .result.i = 13 },
//		}
	// TODO - cf. issue 192
	// 	The following test doesn't match a pattern, since patmat only considers the current
	// 	character it cannot differentiate if 'b' of "beef" still belongs to the "%s"
	// 	specifier or is the beginning of "bar".
	n += test(log, pm, "foodeadbeefbar", PM_NOMATCH, 0, 0x0);
//		(expect_t[]){{ .type = PMS_STR, .result.s =  "deadbeef" }}

	patmat_destroy(pm);

	return -n;
}

test_case(tc_patmat, "pattern matcher");

static int tc_patmat_error(int log){
	int n;
	char const *patterns[] = {
		"%d ",
	};
	char const *patterns_invalid[] = {
		"%s ",
		"%x",
		"%d"
	};


	n = 0;

	n += check_int(log, test(log, 0x0, "", 0, 0, 0x0), -1);

	/* invalid characters */
	n += check_int(log, patmat_match_char(0x0, -1), PM_NOMATCH);

	/* invalid specifier */
	n += check_ptr(log, patmat_init(patterns_invalid, 1), 0x0);
	n += check_ptr(log, patmat_init(patterns_invalid + 1, 1), 0x0);
	n += check_ptr(log, patmat_init(patterns_invalid + 2, 1), 0x0);

	/* malloc fails */
	tmemory_init();

	// fail patmat_init
	tmalloc_fail_at = 1;
	n += check_ptr(log, patmat_init(0x0, 0), 0x0);

	// fail pattern_init 1st
	tmalloc_fail_at = 2;
	n += check_ptr(log, patmat_init(patterns, 1), 0x0);

	// fail pattern_init 2nd
	tmalloc_fail_at = 3;
	n += check_ptr(log, patmat_init(patterns, 1), 0x0);

	// fail pattern_init 3rd
	tmalloc_fail_at = 4;
	n += check_ptr(log, patmat_init(patterns, 1), 0x0);

	tmemory_reset();

	return -n;
}

test_case(tc_patmat_error, "pattern matcher error");
