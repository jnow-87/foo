#include <config/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/term.h>
#include <sys/list.h>
#include <cmd/cmd.h>


/* macros */
#define MAX_LINE_LEN	80


/* static/local prototypes */
static void cmd_exec(char *line, size_t len);
static size_t readline(char *line, size_t n);
static int strsplit(char *line, int *_argc, char ***_argv);
static int redirect_init(FILE *fp, char *file);
static int redirect_revert(FILE *fp, int fd_revert);


/* external variables */
extern cmd_t __cmds_start[],
			 __cmds_end[];


/* static variables */
static cmd_t *cmd_lst = 0x0;


/* global functions */
int main(int argc, char **argv){
	char line[32];
	size_t len;
	term_cfg_t tc;
	cmd_t *cmd;


	/* enable terminal echo */
	ioctl(0, IOCTL_CFGRD, &tc, sizeof(term_cfg_t));
	tc.flags |= TF_ECHO;
	ioctl(0, IOCTL_CFGWR, &tc, sizeof(term_cfg_t));

	/* init command list */
	for(cmd=__cmds_start; cmd!=__cmds_end; cmd++){
		if(cmd->exec == 0x0){
			printf("Null pointer command callback for \"%s\", ignoring command\n", cmd->name);
			continue;
		}

		list_add_tail(cmd_lst, cmd);
	}

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
	cmd_t *cmd;
	term_cfg_t cfg;
	f_mode_t mode;


	/* backup stdin config */
	ioctl(0, IOCTL_CFGRD, &cfg, sizeof(term_cfg_t));
	fcntl(0, F_MODE_GET, &mode, sizeof(f_mode_t));

	/* process command line */
	if(strsplit(line, &argc, &argv)){
		printf("error parsing line\n");
		goto end_0;
	}

	cmd = list_find_str(cmd_lst, name, argv[0]);

	if(cmd == 0x0){
		printf("%s: command not found\n", argv[0]);
		goto end_1;
	}

	errno = 0;

	/* check for output redirection */
	stdout_dup = -1;

	for(i=0; i<argc; i++){
		if(argv[i][0] == '>' && ++i < argc){
			stdout_dup = redirect_init(stdout, argv[i]);

			if(stdout_dup < 0){
				printf("redirecting output to %s failed, errno %s\n", argv[i], strerror(errno));
				goto end_1;
			}
		}
	}

	/* execute command */
	(void)cmd->exec((stdout_dup != -1 ? argc - 2 : argc), argv);

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
	/* restore stdin config */
	ioctl(0, IOCTL_CFGWR, &cfg, sizeof(term_cfg_t));
	fcntl(0, F_MODE_SET, &mode, sizeof(f_mode_t));

	return;
}

static size_t readline(char *line, size_t n){
	size_t i;
	term_err_t terr;


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
	if(errno & E_IO){
		ioctl(0, IOCTL_STATUS, &terr, sizeof(term_err_t));
		printf("readline I/O error: errno %#x, term error %#x\n", errno, terr);
		errno = 0;
	}

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
	int r,
		fd_dup,
		fd_redir;
	stat_t f_stat;


	r = stat(file, &f_stat);

	if((r != 0 && (errno & ~E_UNAVAIL)) || (r == 0 && f_stat.type != FT_REG && f_stat.type != FT_CHR))
		goto_errno(err_0, E_INVAL);

	if(fflush(fp) != 0)
		goto err_0;

	fd_dup = dup(fileno(fp));

	if(fd_dup < 0)
		goto err_0;

	fd_redir = open(file, O_WRONLY | O_CREAT);

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

#ifdef CONFIG_INIT_HELP
static int help(int argc, char **argv){
	size_t line_len,
		   len;
	cmd_t *cmd;



	line_len = 0;

	list_for_each(cmd_lst, cmd){
		if(cmd->name != 0 && cmd->name[0] != '\0' && cmd->exec != 0x0){
			len = strlen(cmd->name) + 1;

			if(line_len + len > MAX_LINE_LEN){
				printf("\n");
				line_len = 0;
			}

			line_len += len;

			printf("%s\t", cmd->name);
		}
	}

	printf("\n");

	return 0;
}

command("help", help);
#endif // CONFIG_INIT_HELP
