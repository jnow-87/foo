/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef OPT_H
#define OPT_H


#include <sys/types.h>
#include <config/config.h>


/* macros */
#define DEFAULT_KERNEL_IMAGE	CONFIG_X86EMU_KERNEL_IMAGE
#define DEFAULT_APP_BINARY		CONFIG_X86EMU_APP_BINARY
#define DEFAULT_ROOTFS			CONFIG_X86EMU_FS_EXPORT_ROOT
#define DEFAULT_INFO			false
#define DEFAULT_VERBOSITY		0
#define DEFAULT_APP_MODE		AM_NONINTERACTIVE
#define DEFAULT_STATS_FD		-1

#define PROGNAME				"x86emu"


/* types */
typedef enum{
	AM_INTERACTIVE = 0x1,
	AM_NONINTERACTIVE = 0x2,
	AM_ALWAYS = 0x3,
} app_mode_t;

typedef struct{
	char *kernel_image,
		 *app_binary;

	char *rootfs;

	bool info;
	unsigned int verbosity;
	int stats_fd;

	app_mode_t app_mode;
} opts_t;


/* prototypes */
int opts_parse(int argc, char **argv);


/* external variables */
extern opts_t opts;


#endif // OPT_H
