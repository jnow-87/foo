/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/term.h>
#include <sys/ioctl.h>
#include <sys/stack.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shell/cmd.h>
#include <shell/shell.h>


/* macros */
#define MAX_LINE_LEN	32
#define MAX_FILE_NAME	8


/* types */
typedef struct stream_stack{
	struct stream_stack *next;
	FILE *stream;
	size_t line;
	char file[MAX_FILE_NAME];
} stream_stack;


/* local/static prototypes */
static size_t readline(FILE *stream, char *line, size_t n);
static int strsplit(char *line, int *_argc, char ***_argv);


/* global variables */
char shell_file[MAX_FILE_NAME];
size_t shell_line = 0;


/* global functions */
void shell(char const *prompt){
	char buf[MAX_LINE_LEN];
	int argc;
	char **argv;
	int exec_err;
	FILE *stream;
	stat_t f_stat;
	stream_stack *streams,
				 *stackp;
	term_cfg_t tc;


	/* init */
	exec_err = 0;
	stream = stdin;
	streams = 0x0;
	shell_line = 0;
	strncpy(shell_file, "stdin", MAX_FILE_NAME);

	// enable terminal echo
	ioctl(0, IOCTL_CFGRD, &tc, sizeof(term_cfg_t));
	tc.flags |= TF_ECHO;
	ioctl(0, IOCTL_CFGWR, &tc, sizeof(term_cfg_t));

	// init commands
	cmd_init();

	/* main loop */
	while(1){
		/* shell prompt */
		if(stream == stdin){
			fputs(prompt, stdout);
			fflush(stdout);
		}

		/* read input */
		shell_line++;

		if(exec_err || stream == 0x0 || readline(stream, buf, MAX_LINE_LEN) == 0){
			exec_err = 0;
			stackp = stack_pop(streams);

			if(stackp){
				fclose(stream);

				stream = stackp->stream;
				strncpy(shell_file, stackp->file, MAX_FILE_NAME);
				shell_line = stackp->line;

				free(stackp);
			}

			continue;
		}

		/* process command line */
		if(strsplit(buf, &argc, &argv) < 0){
			SHELL_ERROR("error parsing line \"%s\"\n", strerror(errno));
			continue;
		}

		/* check if argv[0] is a script file otherwise assume a builtin command */
		if(stat(argv[0], &f_stat) == 0 && f_stat.type == FT_REG){
			// push current stream to stack
			stackp = malloc(sizeof(stream_stack));

			if(stackp == 0x0){
				SHELL_ERROR("out of memory\n");
				goto iter_clean;
			}

			stackp->stream = stream;
			strncpy(stackp->file, shell_file, MAX_FILE_NAME);
			stackp->line = shell_line;

			stack_push(streams, stackp);

			// switch stream to script
			stream = fopen(argv[0], "r");

			if(stream == 0x0)
				SHELL_ERROR("error opening script %s \"%s\"\n", argv[0], strerror(errno));

			// update globals
			strncpy(shell_file, argv[0], MAX_FILE_NAME);
			shell_line = 0;
		}
		else
			exec_err = cmd_exec(argc, argv);

		/* cleanup */
iter_clean:
		for(--argc; argc>=0; argc--)
			free(argv[argc]);

		free(argv);
	}
}


/* local functions */
static size_t readline(FILE *stream, char *line, size_t n){
	size_t i;
	int r;
	term_err_t terr;


	i = 0;

	while(i < n){
		r = read(fileno(stream), line + i, 1);

		if(r < 0)
			goto err;

		if(r == 0 || line[i] == '\n'){
			line[i] = 0;
			return i;
		}

		if(line[i] == '\r')
			continue;

		i++;
	}


err:
	if(errno & E_IO){
		ioctl(fileno(stream), IOCTL_STATUS, &terr, sizeof(term_err_t));
		printf("readline I/O error: %s (%#x), term error %#x\n", strerror(errno), errno, terr);
		errno = 0;
	}

	if(errno){
		printf("readline error on fd %d: %s (%#x)\n", fileno(stream), strerror(errno), errno);
		errno = 0;
	}

	return 0;
}

static int strsplit(char *line, int *_argc, char ***_argv){
	unsigned int i, j, len, start, arg_len;
	int argc;
	char **argv;


	if(line == 0)
		return_errno(E_INVAL);

	len = strlen(line);

	/* identify number of arguments within cmdline */
	i = 0;
	argc = 0;

	while(i < len){
		/* skip blanks */
		if(line[i] == ' '){
			while(++i < len && line[i] == ' ');

			if(i == len)
				break;
		}

		while(i < len){
			if(line[i] == '"'){
				/* hande quoted text */
				while(++i < len && line[i] != '"'){
					if(line[i] == '\\' && i + 1 < len && line[i + 1] == '"')
						i++;
				}

				if(++i >= len)
					break;
			}
			else{
				/* handle un-quoted text */
				do{
					if(line[i] == ' ' || line[i] == '"')
						break;

					if(line[i] == '\\' && i + 1 < len && line[i + 1] == '"')
						i++;
				}while(++i < len);
			}

			/* check for end of argument */
			if(line[i] == ' ')
				break;
		}

		if(i > len)
			break;

		argc++;
	}

	/* alloc argv */
	argv = malloc(argc * sizeof(char*));

	if(argv == 0)
		return_errno(E_NOMEM);

	/* alloc argv[] and assign strings */
	i = 0;
	argc = 0;
	start = 0;
	while(i < len){
		arg_len = 0;

		/* skip blanks */
		if(line[i] == ' '){
			while(++i < len && line[i] == ' ');

			if(i == len)
				break;
		}

		start = i;

		while(i < len){
			if(line[i] == '"'){
				/* handle quoted text */
				while(++i < len && line[i] != '"'){
					if(line[i] == '\\' && i + 1 < len){
						switch(line[i + 1]){
						case '"':
						case 'n':
						case 'r':
						case 't':
						case '\\':
							i++;
							break;

						default:
							break;
						};
					}

					arg_len++;
				}

				if(++i >= len)
					break;
			}
			else{
				/* handle un-quoted text */
				do{
					if(line[i] == ' ' || line[i] == '"')
						break;

					if(line[i] == '\\' && i + 1 < len){
						switch(line[i + 1]){
						case '"':
						case 'n':
						case 'r':
						case 't':
						case '\\':
							i++;
							break;

						default:
							break;
						};
					}

					arg_len++;
				}while(++i < len);
			}

			/* check for end of argument */
			if(line[i] == ' ')
				break;
		}

		if(i > len)
			break;

		/* allocated string */
		argv[argc] = malloc(arg_len + 1);

		if(argv[argc] == 0x0){
			while(--argc >= 0)
				free(argv[argc]);
			free(argv);

			return -E_NOMEM;
		}

		/* copy content from cmdline to argv[] */
		j = 0;
		for(; start<i; start++){
			if(line[start] == '"')
				continue;

			if(line[start] == '\\'){
				start++;

				switch(line[start]){
				case '"':
					argv[argc][j++] = '"';
					break;

				case 'n':
					argv[argc][j++] = '\n';
					break;

				case 'r':
					argv[argc][j++] = '\r';
					break;

				case 't':
					argv[argc][j++] = '\t';
					break;

				case '\\':
					argv[argc][j++] = '\\';
					break;

				default:
					start--;
					argv[argc][j++] = line[start];
					break;
				};
			}
			else
				argv[argc][j++] = line[start];
		}

		argv[argc][j] = 0;
		argc++;
	}

	*_argv = argv;
	*_argc = argc;

	return 0;
}
