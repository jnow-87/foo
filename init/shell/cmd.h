/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef INIT_CMD_H
#define INIT_CMD_H


#include <sys/compiler.h>


/* macros */
#define command(_name, _exec) \
	static char const cmd_name_##_exec[]  __used = _name; \
	static cmd_t _cmd_call_##_exec __linker_array(".commands") = { \
		.name = cmd_name_##_exec, \
		.exec = _exec, \
	}


/* types */
typedef struct cmd_t{
	struct cmd_t *prev,
				 *next;

	char const *name;
	int (*exec)(int argc, char **argv);
} cmd_t;


/* prototypes */
void cmd_init(void);
int cmd_exec(int argc, char **argv);


#endif // INIT_CMD_H
