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
#include <test/test.h>
#include <test/memory.h>


/* types */
typedef struct{
	patmat_spec_type_t type;
	union{
		char c;
		int i;
		char *s;
	} result;
} expect_t;


/* local/static prototypes */
static int test(patmat_t *pm, char const *input, ssize_t retval, size_t nexpect, expect_t *expect);


/* local functions */
TEST(patmat){
	int r = 0;
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


	pm = patmat_init(patterns, sizeof_array(patterns));

	/* special cases */
	r |= TEST_INT_EQ(test(pm, "19 ", 0, 1, (expect_t[]){{ .type = -1, .result.i = 19 }}), -1);
	r |= test(pm, "1+", PM_NOMATCH, 0, 0x0);
	r |= test(pm, "", PM_NOMATCH, 0, 0x0);

	/* no match */
	r |= test(pm, "foo", PM_NOMATCH, 0, 0x0);

	/* match */
	r |= test(pm, "19 ",		0, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 19 }});
	r |= test(pm, "13f00",		2, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 13 }});
	r |= test(pm, "f0011 ",		3, 1, (expect_t[]){{ .type = PMS_INT, .result.i = 11 }});
	r |= test(pm, "foofoobar",	6, 1, (expect_t[]){{ .type = PMS_STR, .result.s = "foo" }});
	r |= test(pm, "fooxbar",	6, 1, (expect_t[]){{ .type = PMS_STR, .result.s = "x" }});
	r |= test(pm, "foot ",		9, 1, (expect_t[]){{ .type = PMS_CHAR, .result.c = 't' }});

	r |= test(pm, "17f0111 ", 4, 2, (expect_t[]){
		{ .type = PMS_INT, .result.i = 17 },
		{ .type = PMS_INT, .result.i = 111 },
	});

	r |= test(pm, "foo13bbar19 ", 5, 2, (expect_t[]){
		{ .type = PMS_INT, .result.i = 13 },
		{ .type = PMS_INT, .result.i = 19 },
	});

	r |= test(pm, "xfoo13bar", 7, 2, (expect_t[]){
		{ .type = PMS_CHAR, .result.c = 'x' },
		{ .type = PMS_INT, .result.i = 13 },
	});

	r |= test(pm, "footestbatefoo17 ", 8, 3, (expect_t[]){
		{ .type = PMS_STR, .result.s = "test" },
		{ .type = PMS_CHAR, .result.c = 'e' },
		{ .type = PMS_INT, .result.i = 17 },
	});

	/* open issues */
	// TODO - cf. issue 196
	// 	The following test matches even though it contains a specifier
//	r |= test(pm, "%d ", PM_NOMATCH, 0, 0x0);

	// TODO - cf. issue 193
	// 	The following test doesn't match a pattern, since patmat is not able to match two
	// 	consecutive specifiers.
	r |= test(pm, "f13 ", PM_NOMATCH, 0, 0x0);
//		(expect_t[]){
//			{ .type = PMS_CHAR, .result.c = 'f' },
//			{ .type = PMS_INT, .result.i = 13 },
//		}
	// TODO - cf. issue 192
	// 	The following test doesn't match a pattern, since patmat only considers the current
	// 	character it cannot differentiate if 'b' of "beef" still belongs to the "%s"
	// 	specifier or is the beginning of "bar".
	r |= test(pm, "foodeadbeefbar", PM_NOMATCH, 0, 0x0);
//		(expect_t[]){{ .type = PMS_STR, .result.s =  "deadbeef" }}

	patmat_destroy(pm);

	return -r;
}

TEST(patmat_error){
	int r = 0;
	char const *patterns[] = {
		"%d ",
	};
	char const *patterns_invalid[] = {
		"%s ",
		"%x",
		"%d"
	};


	r |= TEST_INT_EQ(test(0x0, "", 0, 0, 0x0), -1);

	/* invalid characters */
	r |= TEST_INT_EQ(patmat_match_char(0x0, -1), PM_NOMATCH);

	/* invalid specifier */
	r |= TEST_PTR_EQ(patmat_init(patterns_invalid, 1), 0x0);
	r |= TEST_PTR_EQ(patmat_init(patterns_invalid + 1, 1), 0x0);
	r |= TEST_PTR_EQ(patmat_init(patterns_invalid + 2, 1), 0x0);

	/* malloc fails */
	// fail patmat_init
	memmock_alloc_fail = 0;
	r |= TEST_PTR_EQ(patmat_init(0x0, 0), 0x0);

	// fail pattern_init 1st
	memmock_alloc_fail = 1;
	r |= TEST_PTR_EQ(patmat_init(patterns, 1), 0x0);

	// fail pattern_init 2nd
	memmock_alloc_fail = 2;
	r |= TEST_PTR_EQ(patmat_init(patterns, 1), 0x0);

	// fail pattern_init 3rd
	memmock_alloc_fail = 3;
	r |= TEST_PTR_EQ(patmat_init(patterns, 1), 0x0);

	return -r;
}

static int test(patmat_t *pm, char const *input, ssize_t retval, size_t nexpect, expect_t *expect){
	int r = 0;
	ssize_t index = (retval < 0) ? -1 : retval;
	void *results[nexpect];


	if(pm == 0x0)
		return -1;

	patmat_reset(pm);

	TEST_LOG("match \"%s\"\n", input);

	r |= TEST_INT_EQ(patmat_match_string(pm, input), retval);
	r |= TEST_INT_EQ(patmat_get_results(pm, results), index);

	for(size_t i=0; index>=0 && i<nexpect; i++){
		switch(expect[i].type){
		case PMS_INT:	r |= TEST_INT_EQ(PATMAT_RESULT_INT(results, i), expect[i].result.i); break;
		case PMS_CHAR:	r |= TEST_INT_EQ(PATMAT_RESULT_CHAR(results, i), expect[i].result.c); break;
		case PMS_STR:	r |= TEST_STR_EQ(PATMAT_RESULT_STR(results, i), expect[i].result.s); break;
		default:		r |= 1; break;
		}
	}

	return -r;
}
