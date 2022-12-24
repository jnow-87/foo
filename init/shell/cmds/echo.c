/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <getopt.h>
#include <sys/string.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<args>"
#define OPTS \
	"-e", "evaluate escape characters", \
	"-n", "skip trailing newline", \
	"-h", "print this help message"

#define DEFAULT_OPTS() (opts_t){ \
	.newline = true, \
	.escape = false, \
}


/* types */
typedef struct{
	bool newline,
		 escape;
} opts_t;


/* local/static prototypes */
static int echo(char *s, bool last);
static char esc(char **s);
static char esc_num(char **s, uint8_t n, uint8_t base);


/* static variables */
static opts_t opts;


/* local functions */
static int exec(int argc, char **argv){
	char opt;


	opts = DEFAULT_OPTS();

	/* check options */
	while((opt = getopt(argc, argv, "enh")) != -1){
		switch(opt){
		case 'e':	opts.escape = true; break;
		case 'n':	opts.newline = false; break;
		case 'h':	return CMD_HELP(argv[0], 0x0);
		default:	return CMD_HELP(argv[0], "");
		}
	}

	/* echo non-option arguments */
	for(int i=optind; i<argc; i++){
		if(echo(argv[i], i + 1 >= argc) != 0)
			return -ERROR("write");
	}

	if(opts.newline)
		fputc('\n', stdout);

	fflush(stdout);

	return errno ? -ERROR("flush") : 0;
}

command("echo", exec);


static int echo(char *s, bool last){
	if(opts.escape){
		for(; s[0]!=0; s++){
			if(s[0] == '\\' && s[1] != 0){
				s += 1;
				fputc(esc(&s), stdout);
			}
			else
				fputc(s[0], stdout);
		}
	}
	else
		fputs(s, stdout);

	if(!last)
		fputc(' ', stdout);

	return -errno;
}

static char esc(char **s){
	switch(**s){
	case 'e':	return '\033';
	case 'a':	return '\a';
	case 't':	return '\t';
	case 'b':	return '\b';
	case 'r':	return '\r';
	case 'n':	return '\n';
	case 'v':	return '\v';
	case 'f':	return '\f';
	case '\\':	return '\\';
	case 'x':	return esc_num(s, 2, 16);
	case '0':	return esc_num(s, 3, 8);
	default:	(*s)--; return '\\';
	}
}

static char esc_num(char **s, uint8_t n, uint8_t base){
	char num[n + 1];
	char *end;
	char x;


	strncpy(num, *s + 1, n);
	num[n] = 0;

	x = strtol(num, &end, base);
	*s += end - num;

	return x;
}
