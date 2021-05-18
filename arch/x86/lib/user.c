/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/opts.h>
#include <lib/init.h>
#include <lib/time.h>
#include <sys/compiler.h>
#include <sys/string.h>


/* types */
typedef struct{
	char const *name;
	int (*hdlr)(char const *line);
} user_cmd_t;


/* local/static prototypes */
static void user_hdlr(int sig);
static void cont_hdlr(int sig);

static int cmd_help(char const *line);
static int cmd_time(char const *line);

static char *readline(void);


/* static variables */
static user_cmd_t cmds[] = {
	{ .name = "help",	.hdlr = cmd_help },
	{ .name = "time",	.hdlr = cmd_time },
};


/* global functions */
static int init(void){
	if(!x86_opts.interactive)
		return 0;

	lnx_sigaction(CONFIG_TEST_INT_USR_SIG, user_hdlr, 0x0);
	lnx_sigaction(CONFIG_TEST_INT_CONT_SIG, cont_hdlr, 0x0);

	LNX_DEBUG("waiting for start signal\n");
	lnx_pause();
	LNX_DEBUG("starting init\n");

	return 0;
}

lib_init(0, init);


/* local functions */
static void user_hdlr(int sig){
	char const *line;
	size_t i;
	int r;
	user_cmd_t *cmd;


	cmd = 0x0;
	line = readline();

	if(line == 0x0)
		return;

	for(i=0; i<sizeof_array(cmds); i++){
		if(strncmp(line, cmds[i].name, strlen(cmds[i].name)) == 0){
			cmd = cmds + i;
			break;
		}
	}

	if(cmd != 0x0){
		line += strlen(cmd->name);
		while(*line == ' ') line++;

		r = cmd->hdlr(line);

		if(r != 0)
			lnx_printf("cmd failed with %d, errno: %s\n", r, strerror(errno));
	}
	else
		lnx_printf("invalid command, try 'help' to get a summary\n");
}

static void cont_hdlr(int sig){
	LNX_DEBUG("recv continue signal\n");
}

static int cmd_help(char const *line){
	lnx_printf(
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		, "help", "print this help message"
		, "time", "read the current brickos time"
	);

	return 0;
}

static int cmd_time(char const *line){
	int r;
	time_t t;


	r = time(&t);

	if(r != 0)
		return -1;

	lnx_printf("time: %us %ums %uus\n", t.s, t.ms, t.us);

	return 0;
}

static char *readline(void){
	size_t i;
	static char line[64];


	for(i=0; i<63; i++){
		lnx_read(CONFIG_TEST_INT_USR_PIPE_RD, line + i, 1);

		if(line[i] == '\r')
			i--;

		if(line[i] == '\n')
			break;
	}

	if(line[i] != '\n'){
		lnx_printf("command line limit is 64 characters\n");
		return 0x0;
	}

	line[i] = 0;

	return line;
}
