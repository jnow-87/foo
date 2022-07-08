/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include "opts.h"
#include "user.h"


/* global variables */
opts_t opts = { 0 };


/* local/static prototypes */
static int help(char const *app_name, char const *err, ...);


/* global functions */
int opts_parse(int argc, char **argv){
	int opt;
	int long_optind;
	struct option const long_opt[] = {
		{ .name = "output-file",	.has_arg = required_argument,	.flag = 0,	.val = 'o' },
		{ .name = "vertical",		.has_arg = no_argument,			.flag = 0,	.val = 'v' },
		{ .name = "help",			.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0}
	};


	/* parse arguments */
	while((opt = getopt_long(argc, argv, ":f:s:H:vh", long_opt, &long_optind)) != -1){
		switch(opt){
		case 'o':	opts.source = optarg; break;
		case 'v':	opts.vertical = true; break;
		case 'h':	return help(argv[0], 0x0);

		case ':':	return help(argv[0], "missing argument to \"%s\"", argv[optind - 1]);
		case '?':	return help(argv[0], "invalid option \"%s\"", argv[optind - 1]);
		default:	return help(argv[0], "unknown error");
		}
	}

	/* sanity checks */
	if(optind >= argc)
		return help(argv[0], "missing input-file\n");

	opts.font = argv[optind++];

	if(optind - argc)
		return help(argv[0], "invalid option \"%s\"", argv[optind]);

	if(opts.font == 0x0 || opts.source == 0x0)
		return help(argv[0], "missing arguments\n");

	return 0;
}


/* local functions */
static int help(char const *app_name, char const *err, ...){
	va_list lst;


	if(err != 0x0 && *err != 0){
		va_start(lst, err);

		vprintf(err, lst);
		printf("\n\n");

		va_end(lst);
	}

	printf(
		"usage: %s [<options>] <font>\n"
		"\n"
		"    Convert the given font to a binary representation.\n"
		"\n"
		"Arguments:\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		,
		app_name,
		"-o, --output-file=<file>", "output c file",
		"-v, --vertical", "output font flipped sideways",
		"-h, --help", "print this help message"
	);

	if(err == 0x0)
		exit(0);

	return -1;
}
