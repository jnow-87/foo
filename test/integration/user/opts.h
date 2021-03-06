/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef OPT_H
#define OPT_H


#include <config/config.h>


/* macros */
#define DEFAULT_KERNEL_IMAGE	CONFIG_TEST_INT_KERNEL_IMAGE
#define DEFAULT_APP_BINARY		CONFIG_TEST_INT_APP_BINARY
#define DEFAULT_VERBOSITY		0
#define DEFAULT_APP_MODE		AM_NONINTERACTIVE
#define DEFAULT_STATS_FD		-1

#define PROGNAME				"itest"


/* types */
typedef enum{
	AM_INTERACTIVE = 0x1,
	AM_NONINTERACTIVE = 0x2,
	AM_ALWAYS = 0x3,
} app_mode_t;

typedef struct{
	char *kernel_image,
		 *app_binary;

	unsigned int verbosity;
	int stats_fd;

	app_mode_t app_mode;
} opts_t;


/* prototypes */
int opts_parse(int argc, char **argv);


/* external variables */
extern opts_t opts;


#endif // OPT_H
