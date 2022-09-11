/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <lib/stdio.h>
#include <sys/types.h>
#include <sys/ctype.h>


/* types */
typedef enum{
	OERR_INVAL = 0,
	OERR_MISS,
} err_t;


/* local/static prototypes */
static char getarg(char const *arg, char const *type, int argc, char **argv);
static char traverse(char const *arg, uint8_t consume);
static char error(char const *arg, err_t type);


/* global variables */
char optopt = 0;			// option character that caused an error
int opterr = 1;				// getopt error messages are only printed non-zero
int  optind = 1;			// index of the next argv element to parse
char const *optarg = 0x0;	// an options argument if one is available and expected


/* static variables */
static size_t charind = 1;


/* global functions */
void getopt_reset(void){
	opterr = 1;
	optind = 1;
}

char getopt(int argc, char **argv, char const *optstr){
	char *arg;
	size_t i;


	// scanning modes are not supported
	if(optstr[0] == '-' || optstr[0] == '+')
		return -1;

	if(optstr[0] == ':')
		opterr = -1;

	/* check for end of options, i.e. "-", "--" or non-option */
	if(optind >= argc)
		return -1;

	arg = argv[optind];

	if(arg[0] != '-')
		return -1;

	if(arg[1] == 0 || (arg[1] == '-' && arg[2] == 0)){
		optind++;

		return -1;
	}

	/* parse option */
	optopt = arg[charind];

	for(i=0; optstr[i]!=0; i++){
		if(isalpha(optstr[i]) &&  optstr[i] == optopt)
			return getarg(arg, optstr + i + 1, argc, argv);
	}

	return error(arg, OERR_INVAL);
}


/* local functions */
static char getarg(char const *arg, char const *type, int argc, char **argv){
	/* option without argument */
	if(type[0] != ':')
		return traverse(arg, 0);

	/* argument is part of current argv element */
	optarg = arg + charind + 1;

	if(optarg[0] != 0)
		return traverse(arg, 1);

	/* argument might be optional */
	optarg = 0x0;

	if(type[1] == ':')
		return traverse(arg, 0);

	/* argument might be the next argv element */
	optarg = argv[optind + 1];

	if(optind + 1 < argc)
		return traverse(arg, 2);

	return error(arg, OERR_MISS);
}

static char traverse(char const *arg, uint8_t consume){
	charind++;

	if(arg[charind] != 0 && consume == 0)
		return optopt;

	optind += (consume == 0) ? 1 : consume;
	charind = 1;

	return optopt;
}

static char error(char const *arg, err_t type){
	static char const *msg[] = {
		"invalid option",
		"option requires an argument",
	};


	if(opterr > 0)
		fprintf(stderr, "%s -- '%c'\n", msg[type], optopt);

	(void)traverse(arg, 0);

	return (opterr < 0 && type == OERR_MISS) ? ':' : '?';
}
