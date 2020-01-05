/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/string.h>
#include <sys/patmat.h>
#include <testing/testcase.h>


/* local functions */
static int tc_patmat(int log){
	unsigned int n;
	char const *patterns_invalid[] = {
		"%d"
	};
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
	};
	void *results[3];
	size_t idx;
	patmat_t *pm;


	n = 0;

	pm = patmat_init(patterns_invalid, sizeof_array(patterns_invalid));
	n += check_ptr(log, pm, 0x0);

	pm = patmat_init(patterns, sizeof_array(patterns));

	if(pm == 0x0){
		tlog(log, "init patmat failed\n");
		return -1;
	}

	n += exit_on_error(check_int(log, patmat_match_string(pm, ""), PM_NOMATCH));
	// TODO - cf. issue 196
	// 	The following test matches even though it contains a specifier
//	n += exit_on_error(check_int(log, patmat_match_string(pm, "%d "), PM_NOMATCH));
	n += exit_on_error(check_int(log, patmat_match_string(pm, "foo"), PM_NOMATCH));

	n += exit_on_error(check_int(log, patmat_match_string(pm, "19 "), 0)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 0));
	n += check_int(log, *((int*)(results[0])), 19);

	// TODO - cf. issue 193
	// 	The following test doesn't match a pattern, since patmat is not able to match two
	// 	consecutive specifiers.
	n += exit_on_error(check_int(log, patmat_match_string(pm, "f13 "), PM_NOMATCH)); idx = patmat_get_results(pm, results);
//	n += check_int(log, *((int*)(results[0])), (int)'f');
//	n += check_int(log, *((int*)(results[1])), 13);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "13f00"), 2)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 2));
	n += check_int(log, *((int*)(results[0])), 13);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "f0011 "), 3)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 3));
	n += check_int(log, *((int*)(results[0])), 11);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "17f0111 "), 4)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 4));
	n += check_int(log, *((int*)(results[0])), 17);
	n += check_int(log, *((int*)(results[1])), 111);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "foo13bbar19 "), 5)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 5));
	n += check_int(log, *((int*)(results[0])), 13);
	n += check_int(log, *((int*)(results[1])), 19);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "foofoobar"), 6)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 6));
	n += check_str(log, , results[0], "foo");

	n += exit_on_error(check_int(log, patmat_match_string(pm, "fooxbar"), 6)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 6));
	n += check_str(log, , results[0], "x");

	// TODO - cf. issue 192
	// 	The following test doesn't match a pattern, since patmat only considers the current
	// 	character it cannot differentiate if 'b' of "beef" still belongs to the "%s"
	// 	specifier or is the beginning of "bar".
	n += exit_on_error(check_int(log, patmat_match_string(pm, "foodeadbeefbar"), PM_NOMATCH)); idx = patmat_get_results(pm, results);
//	n += check_str(log, , results[0], "deadbeef");

	n += exit_on_error(check_int(log, patmat_match_string(pm, "xfoo13bar"), 7)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 7));
	n += check_int(log, *((int*)(results[0])), (int)'x');
	n += check_int(log, *((int*)(results[1])), 13);

	n += exit_on_error(check_int(log, patmat_match_string(pm, "foot "), 9)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 9));
	n += check_int(log, *((int*)(results[0])), (int)'t');

	n += exit_on_error(check_int(log, patmat_match_string(pm, "footestbatefoo17 "), 8)); idx = patmat_get_results(pm, results);
	n += exit_on_error(check_int(log, idx, 8));
	n += check_str(log, , results[0], "test");
	n += check_int(log, *((int*)(results[1])), (int)'e');
	n += check_int(log, *((int*)(results[2])), 17);

	patmat_destroy(pm);

	return -n;
}

test_case(tc_patmat, "pattern matcher");
