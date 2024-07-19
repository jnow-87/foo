/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/limits.h>
#include <sys/ctype.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/term.h>
#include <sys/ioctl.h>
#include <sys/stack.h>
#include <sys/escape.h>
#include <sys/stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline.h>
#include <shell/cmd.h>
#include <shell/shell.h>


/* types */
typedef struct shell_stack_t{
	struct shell_stack_t *next;

	FILE *stream;
	size_t line;
	char file[NAME_MAX];
} shell_stack_t;


/* local/static prototypes */
static int splitargs(char *line);
static int exec(char *line, size_t argc);

static char *strchr_esc(char *s, char c);

static int isquote(int c);
static int isshesc(int c);
static int nonblank(int c);


/* global variables */
char const *shell_file;
size_t shell_line;


/* global functions */
int shell_term(char const *prompt){
	term_cfg_t cfg;


	ioctl(0, IOCTL_CFGRD, &cfg);
	cfg.lflags &= ~TLFL_ECHO;
	ioctl(0, IOCTL_CFGWR, &cfg);

	cmd_init();

	history_add("i2ccfg dev/i2c-raw0 master 20");
	history_add("i2ccfg dev/i2c-raw0 slave");
	history_add("cat -n 1 -s 2 dev/i2c-raw0");
	history_add("echo -n \"1234\" > dev/i2c-raw0");
	history_add("echo -n -e \"\\xff\\x00\\x00\\x00\" > dev/gpio-mcp0a");

	while(1){
		write(1, prompt, strlen(prompt));
		(void)shell_script(stdin, "stdin", SH_ONCE);
	}
}

int shell_script(FILE *stream, char const *name, shell_flags_t flags){
	size_t lnum = 1;
	char line[LINE_MAX];
	int argc;
	size_t n;


	while(1){
		shell_line = lnum;
		shell_file = name;

		/* read line */
		n = readline(stream, line, sizeof(line));

		if(n == 0)
			return 0;

		/* exec line */
		argc = splitargs(line);

		if(argc < 0)
			return SHERROR("parsing line");

		strdeesc(line, n, isshesc);

		if(exec(line, argc) != 0 && !(flags & SH_EIGN))
			return -1;

		if(flags & SH_ONCE)
			return 0;

		lnum++;
	}
}

int shell_error(char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);

	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, lst);

	if(errno)
		fprintf(stderr, ": %s", strerror(errno));

	fprintf(stderr, "\n");
	fflush(stderr);

	va_end(lst);

	return -1;
}


/* local functions */
static int history(int argc, char **argv){
	readline_history();

	return 0;
}

command("history", history);

/**
 * \brief	Identify words within line. Words are separated by blanks from each other.
 * 			Single and double quotes combine words. A zero byte is added at the end of
 * 			each word.
 *
 * \param	line	string to parse
 *
 * \return	number of words identified
 */
static int splitargs(char *line){
	int argc = 0;
	char *end;


	for(char *start=line; *start!=0; start=end+1){
		start = (char*)strpchr(start, nonblank);

		if(*start == 0)
			break;

		if(isquote(*start)){
			end = strchr_esc(start + 1, *start);

			if(*end != *start)
				return_errno(E_INVAL);

			*start = ' ';
		}
		else
			end = (char*)strpchr(start + 1, isblank);

		argc++;

		if(*end == 0)
			break;

		*end = 0;
	}

	return argc;
}

static int exec(char *line, size_t argc){
	int r;
	char *argv[argc];
	FILE *script;
	stat_t f_stat;


	/* assign argv */
	for(size_t i=0; i<argc; i++){
		argv[i] = (char*)strpchr(line, nonblank);
		line = (char*)strchr(argv[i], 0) + 1;
	}

	/* check if argv[0] is a script or built-in command */
	if(stat(argv[0], &f_stat) != 0 || f_stat.type != FT_REG)
		return cmd_exec(argc, argv);

	script = fopen(argv[0], "r");

	if(script == 0x0)
		return SHERROR("opening script %s", argv[0]);

	r = shell_script(script, argv[0], SH_EIGN);
	fclose(script);

	return r;
}

static char *strchr_esc(char *s, char c){
	for(; *s!=0; s++){
		if(*s == c)
			break;

		if(*s == '\\' && s[1] != 0)
			s++;
	}

	return s;
}

static int isquote(int c){
	return c == '"' || c == '\'';
}

static int isshesc(int c){
	switch(c){
	case '\\':	return '\\';
	case '"':	return '"';
	default:	return ESC_RESOLVE_NONE;
	}
}

static int nonblank(int c){
	return !isblank(c);
}
