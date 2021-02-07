/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>


/* global variables */
/**
 * \brief	control [cm]alloc failures
 * 			decemented with each call to [cm]alloc if enabled
 * 				 0: [cm]alloc return 0x0 at next call
 * 				-1: mock failure disabled
 */
ssize_t memmock_alloc_fail = -1;
