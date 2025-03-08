/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <options.h>
#include <types.h>


/* local/static prototypes */
static void help(char const *prg_name, char const *msg, ...);


/* global variables */
opt_t options = {
	.ifile_name = 0x0,
	.ofile_name = 0x0,
	.nodes = 0x0,
	.ofile_format = FMT_C,
};


/* global functions */
void opt_parse(int argc, char **argv){
	int opt;
	struct option const long_opt[] = {
		{ .name = "output",	.val = 'o',	.has_arg = required_argument,	.flag = 0x0 },
		{ .name = "format",	.val = 'f',	.has_arg = required_argument,	.flag = 0x0 },
		{ .name = "nodes",	.val = 'n',	.has_arg = required_argument,	.flag = 0x0 },
		{ .name = "help",	.val = 'h',	.has_arg = no_argument,			.flag = 0x0 },
		{ 0, 0, 0, 0}
	};


	/* parse options */
	while((opt = getopt_long(argc, argv, "o:f:n:vh", long_opt, 0x0)) != -1){
		switch(opt){
		case 'h':	help(argv[0], 0x0); break;
		case 'o':	options.ofile_name = optarg; break;
		case 'n':	options.nodes = optarg; break;

		case 'f':
			if(strcmp(optarg, "c") == 0)			options.ofile_format = FMT_C;
			else if(strcmp(optarg, "header") == 0)	options.ofile_format = FMT_HEADER;
			else if(strcmp(optarg, "make") == 0)	options.ofile_format = FMT_MAKE;
			else									help(argv[0], "unknown output file format \"%s\"\n", optarg);

			break;

		case ':':	/* missing argument */
			help(argv[0], "missing argument to \"%s\"\n", argv[optind - 1]);
			break;

		case '?':	/* invalid option */
		default:
			help(argv[0], "invalid option \"%s\"\n", argv[optind - 1]);
			break;
		}
	}

	/* handle non-option arguments */
	if(optind == argc)
		help(argv[0], "missing device tree script\n");

	options.ifile_name = argv[optind++];

	/* check for unhandled arguments */
	if(optind < argc){
		fprintf(stderr, "%d unknown argument(s):", argc - optind);

		while(optind < argc){
			fprintf(stderr, " %s", argv[optind]);
			optind++;
		}

		help(argv[0], "\n");
	}
}


/* local functions */
static void help(char const *prg_name, char const *msg, ...){
	va_list lst;


	if(msg && *msg != 0){
		va_start(lst, msg);
		vprintf(msg, lst);
		va_end(lst);
		fprintf(stderr, "\n");
	}

	fprintf(stderr,
		"usage: %s [options] <device tree script>\n\n"
		"options:\n"
		"    -o, --output      <output file>      output file (default=stdout)\n"
		"    -f, --format      <format>           output file format, either of c, header and make (default=c)\n"
		"    -n, --nodes       <node list>        comma separated list of device tree node names to export\n"
		"                                         (default=<all nodes>)\n"
		"    -h, --help                           print this help message\n"
		,
		prg_name
	);

	exit((msg && *msg != 0) ? 1 : 0);
}
