#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <cmd/cmd.hash.h>

#ifndef BUILD_HOST
#include <sys/term.h>
#endif // BUILD_HOST


/* static/local prototypes */
static void cmd_exec(char *line, size_t len);
static size_t readline(char *line, size_t n);
static int strsplit(char *line, int *_argc, char ***_argv);
static int redirect_init(FILE *fp, char *file);
static int redirect_revert(FILE *fp, int fd_revert);


/* global functions */
int main(int argc, char **argv){
	char line[32];
	size_t len;

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

		if(len == 0)
			continue;

		cmd_exec(line, len);
	}

	return 0;
}


/* local functions */
static void cmd_exec(char *line, size_t len){
	int i;
	int argc;
	char **argv;
	int stdout_dup;
	cmd_t const *cmd;


	/* process command line */
	if(strsplit(line, &argc, &argv)){
		printf("error parsing line\n");
		goto end_0;
	}

	cmd = cmd_lookup(argv[0], strlen(argv[0]));

	if(cmd == 0x0){
		printf("%s: command not found\n", argv[0]);
		goto end_1;
	}

	/* check for output redirection */
	stdout_dup = -1;

	for(i=0; i<argc; i++){
		if(argv[i][0] == '>' && ++i < argc){
			stdout_dup = redirect_init(stdout, argv[i]);

			if(stdout_dup < 0){
				printf("redirecting output to %s failed, errno %#x\n", argv[i], errno);
				goto end_1;
			}
		}
	}

	/* execute command */
	errno = 0;

	if(cmd->exec == 0x0)	printf("%s: command disabled through config\n", argv[0]);
	else					(void)cmd->exec((stdout_dup != -1 ? argc - 2 : argc), argv);

	/* revert output redirection */
	if(stdout_dup != -1){
		if(redirect_revert(stdout, stdout_dup))
			printf("reverting output redirection failed, errno %#x\n", errno);
	}

end_1:
	for(i=0; i<argc; i++)
		free(argv[i]);
	free(argv);

end_0:
	return;
}

static size_t readline(char *line, size_t n){
	size_t i;

#ifndef BUILD_HOST
	term_err_t terr;
#endif // BUILD_HOST


	i = 0;

	while(i < n){
		if(read(0, line + i, 1) != 1)
			goto err;

		if(line[i] == '\r')
			continue;

		if(line[i] == '\n'){
			line[i] = 0;
			return i;
		}

		i++;
	}


err:
#ifndef BUILD_HOST
	if(errno & E_IO){
		ioctl(0, IOCTL_STATUS, &terr, sizeof(term_err_t));
		printf("readline I/O error: errno %#x, term error %#x\n", errno, terr);
		errno = 0;
	}
#endif // BUILD_HOST

	if(errno){
		printf("readline error: errno %#x\n", errno);
		errno = 0;
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

static int redirect_init(FILE *fp, char *file){
	int fd_dup,
		fd_redir;


	if(fflush(fp) != 0)
		goto err_0;

	fd_dup = dup(fileno(fp));

	if(fd_dup < 0)
		goto err_0;

#ifdef BUILD_HOST
	fd_redir = open(file, O_WRONLY | O_CREAT, 0664);
#else
	fd_redir = open(file, O_WRONLY | O_CREAT);
#endif // BUILD_HOST

	if(fd_redir < 0)
		goto err_1;

	if(dup2(fd_redir, fileno(fp)) != fileno(fp))
		goto err_2;

	(void)close(fd_redir);

	return fd_dup;


err_2:
	close(fd_redir);

err_1:
	close(fd_dup);

err_0:
	return -1;
}

static int redirect_revert(FILE *fp, int fd_revert){
	if(dup2(fd_revert, fileno(fp)) != fileno(fp))
		return -1;
	return close(fd_revert);
}
