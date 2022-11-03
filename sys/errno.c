/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/errno.h>


/* global variables */
errno_t errno = 0;

#ifdef CONFIG_EXTENDED_ERRNO
char const *errno_file = "";
unsigned int errno_line = 0;
#endif // CONFIG_EXTENDED_ERRNO
