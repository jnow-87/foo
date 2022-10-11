/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <sys/compiler.h>
#include <lib/getopt.h>
#include <test/test.h>


/* macros */
#define ARGV(...)	((char *[]){ __VA_ARGS__})
#define ARGCV(...)	sizeof_array(ARGV("pname", __VA_ARGS__)), ARGV("pname", __VA_ARGS__)


/* local/static prototypes */
static int test(char const *optstr, int argc, char **argv, char const *optopts, char **optargs);


/* local functions */
TEST(getopt){
	int r = 0;


	r += TEST_INT_EQ(optind, 1);

	/* empty arguments */
	r += test("", ARGCV(), "", ARGV(0x0));

	/* end of options */
	r += test("", ARGCV("-"), "", ARGV(0x0));
	r += test("", ARGCV("--"), "", ARGV(0x0));
	r += test("", ARGCV("-", "foo"), "", ARGV("foo", 0x0));
	r += test("", ARGCV("--", "foo"), "", ARGV("foo", 0x0));
	r += test("i", ARGCV("-", "-i"), "", ARGV("-i", 0x0));
	r += test("i", ARGCV("--", "-i"), "", ARGV("-i", 0x0));
	r += test("i", ARGCV("--l", "-i"), "??i", ARGV(0x0));

	/* options without arguments */
	r += test("ik", ARGCV("-i", "-k"), "ik", ARGV(0x0));
	r += test("ik", ARGCV("-k", "-i"), "ki", ARGV(0x0));
	r += test("ik", ARGCV("-ik"), "ik", ARGV(0x0));
	r += test("ik", ARGCV("-ki"), "ki", ARGV(0x0));
	r += test("ik", ARGCV("-k", "-i", "foo"), "ki", ARGV("foo", 0x0));
	r += test("ik", ARGCV("-k", "-i", "--", "foo"), "ki", ARGV("foo", 0x0));

	/* options with required arguments */
	r += test("a:", ARGCV("-a", "arg"), "a*", ARGV("arg", 0x0));
	r += test("a:", ARGCV("-aarg"), "a*", ARGV("arg", 0x0));

	/* options with optional arguments */
	r += test("o::", ARGCV("-o"), "o", ARGV(0x0));
	r += test("o::", ARGCV("-o", "arg"), "o", ARGV("arg", 0x0));
	r += test("o::", ARGCV("-oarg"), "o*", ARGV("arg", 0x0));

	/* all combined */
	r += test("ika:o::", ARGCV("-k", "-a", "foo", "-i", "-o", "-k"), "ka*iok", ARGV("foo", 0x0));
	r += test("ika:o::", ARGCV("-k", "-a", "foo", "-i", "-obar", "-k", "arg"), "ka*io*k", ARGV("foo", "bar", "arg", 0x0));
	r += test("ika:o::", ARGCV("-ka", "foo", "--", "-i", "-o", "-k"), "ka*", ARGV("foo", "-i", "-o", "-k", 0x0));
	r += test("ika:o::", ARGCV("-kafoo", "--", "-i", "-o", "-k"), "ka*", ARGV("foo", "-i", "-o", "-k", 0x0));

	/* errors */
	// invalid option
	r += test("i", ARGCV("-k"), "?", ARGV(0x0));
	r += test(":i", ARGCV("-k"), "?", ARGV(0x0));

	// missing argument
	r += test("a:", ARGCV("-a"), "?", ARGV(0x0));
	r += test(":a:", ARGCV("-a"), ":", ARGV(0x0));

	/* compatibility */
	r += test("+", ARGCV(), "", ARGV(0x0));
	r += test("-", ARGCV(), "", ARGV(0x0));
	r += test("+ika:o::", ARGCV("-a", "arg"), "", ARGV("-a", "arg", 0x0));
	r += test("-ika:o::", ARGCV("-a", "arg"), "", ARGV("-a", "arg", 0x0));

	return -r;
}

static int test(char const *optstr, int argc, char **argv, char const *optopts, char **optargs){
	int i = 0,
		j = 0,
		r = 0;
	char opt;


	getopt_reset();

	TEST_LOG("optstr: \"%s\" optopts: \"%s\"\n", optstr, optopts);

	/* ensure the expected options are returned */
	while((opt = getopt(argc, argv, optstr)) != -1){
		// option character
		r += TEST_INT_NEQ(optopts[i], 0);
		r += TEST_INT_EQ(opt, optopts[i++]);

		// argument
		if(optopts[i] == '*'){
			r += TEST_PTR_NEQ(optargs[j], 0x0);
			r += TEST_STR_EQ(optarg, optargs[j++]);
			i++;
		}
	}

	/* ensure all expected options have been returned */
	r += TEST_INT_EQ(optopts[i], 0);

	/* ensure remaining argv elements are correct */
	for(i=optind; i<argc; i++, j++){
		r += TEST_PTR_NEQ(argv[i], 0x0);
		r += TEST_PTR_NEQ(optargs[j], 0x0);

		r += TEST_STR_EQ(argv[i], optargs[j]);
	}

	/* ensure all expected arguments have been used */
	r += TEST_PTR_EQ(optargs[j], 0x0);

	return r;
}
