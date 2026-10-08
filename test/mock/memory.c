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
 * 			decremented with each call to [cm]alloc if enabled
 * 				 0: [cm]alloc return 0x0 at next call
 * 				-1: mock failure disabled
 */
ssize_t memmock_alloc_fail_in = -1;

/** \brief	control [cm]alloc failures
 * 			if set, evaluated with each call to [cm]alloc
 * 			return values are treated as
 * 				0: [cm]alloc return 0x0
 * 				-1: mock disabled, sames as being set to 0x0
 * 				>0: [cm]alloc called unchanged
 */
int (*memmock_alloc_fail_callback)(size_t n, size_t size) = 0x0;


/* global functions */
void memmock_reset(void){
	memmock_alloc_fail_callback = 0x0;
	memmock_alloc_fail_in = -1;
}
