/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <arch/x86/hardware.h>
#include <sys/escape.h>
#include <sys/compiler.h>
#include <user/opts.h>


/* macros */
#define DEFAULT(v)			"(default: " STRGIFY(v) ")"


/* local/static prototypes */
static int help(char const *err, ...);


/* global variables */
opts_t opts = {
	.kernel_image = DEFAULT_KERNEL_IMAGE,
	.app_binary = DEFAULT_APP_BINARY,
	.verbosity = DEFAULT_VERBOSITY,
	.stats_fd = DEFAULT_STATS_FD,
	.app_mode = DEFAULT_APP_MODE,
};


/* global functions */
int opts_parse(int argc, char **argv){
	int opt;
	int long_optind;
	struct option const long_opt[] = {
		{ .name = "kernel",			.has_arg = required_argument,	.flag = 0,	.val = 'k' },
		{ .name = "application",	.has_arg = required_argument,	.flag = 0,	.val = 'a' },
		{ .name = "verbose",		.has_arg = optional_argument,	.flag = 0,	.val = 'v' },
		{ .name = "interactive",	.has_arg = no_argument,			.flag = 0,	.val = 'i' },
		{ .name = "stats",			.has_arg = required_argument,	.flag = 0,	.val = 's' },
		{ .name = "help",			.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0}
	};


	/* parse arguments */
	while((opt = getopt_long(argc, argv, ":k:a:v::is:h", long_opt, &long_optind)) != -1){
		switch(opt){
		case 'k':	opts.kernel_image = optarg; break;
		case 'a':	opts.app_binary = optarg; break;
		case 'v':	opts.verbosity = (optarg ? atoi(optarg) : 1); break;
		case 'i':	opts.app_mode = AM_INTERACTIVE; break;
		case 's':	opts.stats_fd = atoi(optarg); break;
		case 'h':	(void)help(0x0); return argc;

		case ':':	return help("missing argument to \"%s\"", argv[optind - 1]);
		case '?':	return help("invalid option \"%s\"", argv[optind - 1]);
		default:	return help("unknown error");
		}
	}

	if(optind - argc){
		help("invalid options \"%s\"", argv[optind]);

		return -1;
	}
	return 0;
}


/* local functions */
static int help(char const *err, ...){
	va_list lst;


	if(err != 0x0 && *err != 0){
		va_start(lst, err);

		printf("%serror%s: ", FG_RED, RESET_ATTR);
		vprintf(err, lst);
		printf("\n\n");

		va_end(lst);
	}

	printf(
		"usage: %s [options]\n"
		"\n"
		"    fork kernel and application binary simulating a hardware between both\n"
		"        hardware-op read/write fileno: %d/%d\n"
		"        hardware-op signals: %d - %d\n"
		"        user read/write fileno: %d/%d\n"
		"        user signal: %d\n"
		"        uart signal: %d\n"
		"\n"
		"Options:\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"\n"
		"    %-25.25s    %s\n"
		, PROGNAME
		, CONFIG_TEST_INT_HW_PIPE_RD
		, CONFIG_TEST_INT_HW_PIPE_WR
		, CONFIG_TEST_INT_HW_SIG, CONFIG_TEST_INT_HW_SIG + X86_INT_PRIOS - 1
		, CONFIG_TEST_INT_USR_PIPE_RD
		, CONFIG_TEST_INT_USR_PIPE_WR
		, CONFIG_TEST_INT_USR_SIG
		, CONFIG_TEST_INT_UART_SIG
		, "-k, --kernel=<image>", "use <image> as kernel " DEFAULT(DEFAULT_KERNEL_IMAGE)
		, "-a, --application=<image>", "use <image> as application binary " DEFAULT(DEFAULT_APP_BINARY)
		, "-i, --interactive", "test execution is under user control " DEFAULT(DEFAULT_APP_MODE)
		, "-v, --verbose[=<level>]", "enable verbose output"
		, "-s, --stats=<fd>", "enable hardware statistics being printed to file descriptor <fd> " DEFAULT(DEFAULT_STATS_FD)
		, "-h, --help", "print this help message"
	);

	return (err == 0x0) ? 0 : -1;
}
