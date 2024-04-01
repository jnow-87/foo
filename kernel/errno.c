/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/devicetree.h>
#include <sys/errno.h>


/* global variables */
errno_t _errno[DEVTREE_ARCH_NCORES] = { 0 };

#ifdef CONFIG_EXTENDED_ERRNO
char const *_errno_file[DEVTREE_ARCH_NCORES] = { 0x0 };
unsigned int _errno_line[DEVTREE_ARCH_NCORES] = { 0 };
#endif // CONFIG_EXTENDED_ERRNO
