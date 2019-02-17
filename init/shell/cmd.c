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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <shell/cmd.h>
#include <shell/shell.h>


/* macros */
#define MAX_LINE_LEN	80


/* local/static prototypes */
static int redirect_init(FILE *fp, char const *file, char const *redir_type);
static int redirect_revert(FILE *fp, int fd_revert);


/* external variables */
extern cmd_t __cmds_start[],
			 __cmds_end[];


/* static variables */
static cmd_t *cmd_lst = 0x0;


/* global functions */
void cmd_init(void){
	cmd_t *cmd;


	/* init command list */
	for(cmd=__cmds_start; cmd!=__cmds_end; cmd++){
		if(cmd->exec == 0x0){
			fprintf(stderr, "Null pointer command callback for \"%s\", ignoring command\n", cmd->name);
			continue;
		}

		list_add_tail(cmd_lst, cmd);
	}
}

int cmd_exec(int argc, char **argv){
	int i,
		r;
	int stdout_dup;
	cmd_t *cmd;


	/* get command */
	cmd = list_find_str(cmd_lst, name, argv[0]);

	if(cmd == 0x0){
		SHELL_ERROR("unknown command \"%s\"\n", argv[0]);
		return -E_INVAL;
	}

	errno = E_OK;

	/* check for output redirection */
	stdout_dup = -1;

	for(i=0; i<argc; i++){
		if(argv[i][0] == '>' && i + 1 < argc){
			stdout_dup = redirect_init(stdout, argv[i + 1], argv[i]);
			i++;

			if(stdout_dup < 0){
				fprintf(stderr, "redirecting output to %s failed, errno %s\n", argv[i], strerror(errno));
				return -errno;
			}
		}
	}

	/* execute command */
	(void)cmd->exec((stdout_dup != -1 ? argc - 2 : argc), argv);

	/* revert output redirection */
	if(stdout_dup != -1){
		r = redirect_revert(stdout, stdout_dup);

		if(r){
			printf("reverting output redirection failed, errno %#x\n", errno);
			return r;
		}
	}

	return 0;
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
		errno = E_OK;

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
	(void)fflush(fp);

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
