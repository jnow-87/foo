/*
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_IOCTL_H
#define SYS_IOCTL_H


/* types */
typedef enum{
	IOCTL_CFGRD = 1,
	IOCTL_CFGWR,
	IOCTL_STATUS,
} ioct_cmd_t;


#endif // SYS_IOCTL_H
