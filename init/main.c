#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <cmd/cmd.hash.h>

#ifndef BUILD_HOST
#include <sys/term.h>
#endif // BUILD_HOST


/* static/local prototypes */
static void cmd_exec(char *line, size_t len);
static size_t readline(char *line, size_t n);
static int strsplit(char *line, int *_argc, char ***_argv);


/* global functions */
int main(int argc, char **argv){
	char line[32];
	size_t len;
	term_err_t terr;

#ifndef BUILD_HOST
	term_cfg_t tc;
#endif // BUILD_HOST


#ifndef BUILD_HOST
	/* enable terminal echo */
	ioctl(0, IOCTL_CFGRD, &tc, sizeof(term_cfg_t));
	tc.flags |= TF_ECHO;
	ioctl(0, IOCTL_CFGWR, &tc, sizeof(term_cfg_t));
#endif // BUILD_HOST

	/* command loop */
	while(1){
		fputs("> ", stdout);
		fflush(stdout);

		len = readline(line, 32);

		if(len == 0){
			if(errno){
				ioctl(0, IOCTL_STATUS, &terr, sizeof(term_err_t));
				printf("readline error: errno %#x, term error %#x\n", errno, terr);
				errno = E_OK;
			}

			continue;
		}

		cmd_exec(line, len);
	}

	return 0;
}


/* local functions */
static void cmd_exec(char *line, size_t len){
	cmd_t const *cmd;
	int argc;
	char **argv;


	if(strsplit(line, &argc, &argv)){
		printf("error parsing line\n");
		return;
	}

	cmd = cmd_lookup(argv[0], strlen(argv[0]));

	if(cmd == 0x0){
		printf("%s: command not found\n", argv[0]);
		return;
	}

	if(cmd->exec == 0x0)	printf("%s: command disabled through config\n", argv[0]);
	else					(void)cmd->exec(argc, argv);

	free(argv);
}

static size_t readline(char *line, size_t n){
	size_t i;


	i = 0;

	while(i < n){
		if(read(0, line + i, 1) != 1)
			return 0;

		if(line[i] == '\r')
			continue;

		if(line[i] == '\n'){
			line[i] = 0;
			return i;
		}

		i++;
	}

	return 0;
}

static int strsplit(char *line, int *_argc, char ***_argv){
	unsigned int i, j, len, start, arg_len;
	int argc;
	char **argv;


	if(line == 0)
		return -1;

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
		return -1;

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
