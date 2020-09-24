/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <include/sys/compiler.h>
#include <user/debug.h>
#include <hardware/hardware.h>
#include <brickos/child.h>
#include <brickos/brickos.h>


/* types */
typedef struct{
	char const *name;
	void (*hdlr)(char const *line);
} user_cmd_t;


/* local/static prototypes */
static void cmd_help(char const *line);
static void cmd_quit(char const *line);
static void cmd_tick(char const *line);
static void cmd_signal(char const *line);
static void cmd_app(char const *line);


/* static variables */
static user_cmd_t cmds[] = {
	{ .name = "help",	.hdlr = cmd_help },
	{ .name = "quit",	.hdlr = cmd_quit },
	{ .name = "tick",	.hdlr = cmd_tick },
	{ .name = "signal",	.hdlr = cmd_signal },
	{ .name = "app",	.hdlr = cmd_app },
};


/* global functions */
void user_input_help(void){
	printf(
		"Starting %s in interactive mode.\n"
		"  In interactive mode both kernel and application are halted\n"
		"  and need to be sent a signal to start execution.\n"
		"\n"
		"  Program information\n"
		"    %s pid: %u\n"
		"    kernel pid: %u\n"
		"    app pid: %u\n"
		"\n"
		"  Command help\n"
		, PROGNAME
		, PROGNAME, getpid()
		, KERNEL->pid
		, APP->pid
	);

	cmd_help("");
}

void user_input_process(void){
	size_t i;
	char *line;
	user_cmd_t *cmd;
	HIST_ENTRY *last;



	/* read a line and add it to the command history */
	last = history_get(history_length);
	line = readline("$> ");

	if(line == 0x0)
		EEXIT("readline failed with %s\n", strerror(errno));

	if(last == 0x0 || strcmp(line, last->line) != 0){
		if(*line != 0)
			add_history(line);

		free(line);

		last = history_get(history_length);
		line = (last == 0x0) ? "" : last->line;
	}

	/* identify the command */
	cmd = 0x0;

	for(i=0; i<sizeof_array(cmds); i++){
		if(strncmp(line, cmds[i].name, strlen(cmds[i].name)) == 0){
			cmd = cmds + i;
			break;
		}
	}

	/* handle the command */
	if(cmd != 0x0){
		line += strlen(cmd->name);
		while(*line == ' ') line++;

		cmd->hdlr(line);
	}
	else
		printf("invalid command, try 'help' to get a summary\n");
}

void user_input_cleanup(void){
	rl_clear_history();
}


/* local functions */
static void cmd_help(char const *line){
	printf(
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		, "help", "print command help information"
		, "quit", "exit the program"
		, "tick [<n>]", "issue <n> (default 1) hardware timer interrupts to kernel"
		, "signal <target>", "send SIGCONT to <target>, either 'kernel' or 'app'"
		, "app <cmd>", "forward <cmd> to the app"
	);
}

static void cmd_quit(char const *line){
	exit(0);
}

static void cmd_tick(char const *line){
	long int n,
			 i;


	n = strtol(line, 0x0, 10);
	n = (n == 0) ? 1 : n;

	for(i=0; i<n; i++){
		printf("tick %u/%u\n", i + 1, n);
		hw_int_request(INT_TIMER, 0x0, HWS_HARDWARE);
		usleep(5);
	}
}

static void cmd_signal(char const *line){
	child_t *tgt;


	tgt = 0x0;

	if(strcmp(line, "kernel") == 0)		tgt = KERNEL;
	else if(strcmp(line, "app") == 0)	tgt = APP;

	if(tgt){
		child_lock(tgt);
		child_signal(tgt, SIGCONT);
		child_unlock(tgt);
	}
	else
		printf("invalid target, use kernel or app\n");
}

static void cmd_app(char const *line){
	child_lock(APP);

	child_signal(APP, CONFIG_TEST_INT_CTRL_SIG);
	child_write(APP, 1, (char*)line, strlen(line));
	child_write(APP, 1, "\n", 1);

	child_unlock(APP);
}
