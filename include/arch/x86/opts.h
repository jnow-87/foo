/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_OPTS_H
#define X86_OPTS_H


#include <sys/types.h>


/* types */
typedef struct{
	unsigned int debug;
	bool interactive;
} x86_opts_t;


/* prototypes */
int x86_opts_parse(void);


/* external variables */
extern x86_opts_t x86_opts;


#endif // X86_OPTS_H
