/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/stdarg.h>
#include <sys/math.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <shell/cmd.h>
#include <shell/shell.h>


/* macros */
#define MAX_LINE_LEN	80


/* local/static prototypes */
static int redirect_init(FILE *fp, char const *file, char const *redir_type);
static int redirect_revert(FILE *fp, int fd_revert);


/* external variables */
extern cmd_t __start_cmds[],
			 __stop_cmds[];


/* static variables */
static cmd_t *cmd_lst = 0x0;


/* global functions */
void cmd_init(void){
	cmd_t *cmd;


	/* init command list */
	for(cmd=__start_cmds; cmd!=__stop_cmds; cmd++){
		if(cmd->exec == 0x0){
			ERROR("missing exec callback for %s, ignoring command", cmd->name);
			continue;
		}

		list_add_tail(cmd_lst, cmd);
	}
}

int cmd_exec(int argc, char **argv){
	int stdout_dup = -1;
	cmd_t *cmd;


	/* get command */
	if(cmd_lst == 0x0)
		return ERROR("no builtin commands");

	cmd = list_find_str(cmd_lst, name, argv[0]);

	if(cmd == 0x0)
		return SHERROR("unknown command %s", argv[0]);

	reset_errno();

	/* check for output redirection */
	for(int i=0; i<argc; i++){
		if(argv[i][0] == '>' && i + 1 < argc){
			stdout_dup = redirect_init(stdout, argv[i + 1], argv[i]);
			i++;

			if(stdout_dup < 0)
				return ERROR("redirecting output to %s", argv[i]);
		}
	}

	/* execute command */
	getopt_reset();

	(void)cmd->exec((stdout_dup != -1 ? argc - 2 : argc), argv);

	(void)fflush(stdout);
	(void)fflush(stderr);

	/* revert output redirection */
	if(stdout_dup != -1){
		if(redirect_revert(stdout, stdout_dup) != 0)
			return ERROR("reverting output redirection");
	}

	return 0;
}

int cmd_help(char const *name, char const *args, char const *error, size_t nopts, ...){
	size_t opt_len = 0;
	char const *opt,
			   *help;
	char fmt[20];
	va_list lst;


	if(error && error[0] != 0){
		ERROR("%s", error);
		printf("\n");
	}

	printf("usage: %s %s%s\n", name, (nopts > 0) ? "[options] " : "", args);

	if(nopts > 0){
		va_start(lst, nopts);

		// determine length of option strings
		for(size_t i=0; i<nopts; i++){
			opt = va_arg(lst, char*);
			help = va_arg(lst, char*);

			opt_len = MAX(opt_len, strlen(opt) + 1);
		}

		va_end(lst);
		snprintf(fmt, sizeof(fmt), "%%%u.%us    %%s\n", opt_len, opt_len);

		// print options
		printf("\noptions:\n");

		va_start(lst, nopts);

		for(size_t i=0; i<nopts; i++){
			opt = va_arg(lst, char*);
			help = va_arg(lst, char*);

			printf(fmt, opt, help);
		}

		va_end(lst);
	}

	return (error != 0x0) ? 1 : 0;
}


/* local functions */
static int redirect_init(FILE *fp, char const *file, char const *redir_type){
	int r,
		fd_dup,
		fd_redir;
	f_mode_t mode;
	stat_t f_stat;


	/* preparation */
	r = stat(file, &f_stat);

	// check if file is a regular file or character device
	if(errno == E_UNAVAIL)
		reset_errno();

	if((r != 0 && errno) || (r == 0 && f_stat.type != FT_REG && f_stat.type != FT_CHR))
		goto_errno(err_0, E_INVAL);

	// remove file depending on redir_type
	if(r == 0 && f_stat.type == FT_REG && strcmp(redir_type, ">") == 0)
		unlink(file);

	// flush target stream
	if(fflush(fp) != 0)
		goto err_0;

	/* perform redirection */
	fd_dup = dup(fileno(fp));

	if(fd_dup < 0)
		goto err_0;

	mode = O_WRONLY | O_CREAT;

	if(strcmp(redir_type, ">>") == 0)
		mode |= O_APPEND;

	fd_redir = open(file, mode);

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
	size_t line_len = 0;
	size_t len;
	cmd_t *cmd;



	list_for_each(cmd_lst, cmd){
		if(cmd->name != 0x0 && cmd->name[0] != '\0' && cmd->exec != 0x0){
			len = strlen(cmd->name) + 1;

			if(line_len + len > MAX_LINE_LEN){
				fputs("\n", stdout);
				line_len = 0;
			}

			line_len += len;

			printf("%s\t", cmd->name);
		}
	}

	fputs("\n", stdout);

	return 0;
}

command("help", help);
#endif // CONFIG_INIT_HELP
