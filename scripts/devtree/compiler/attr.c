/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/types.h>
#include <stdlib.h>
#include <attr.h>
#include <parser.tab.h>


/* global functions */
char const *attr_name(attr_type_t type){
	static char const *names[] = {
		"unknown",
		"compatible",
		"addr",
		"string",
		"int8",
		"int16",
		"int32",
		"int64",
		"size",
		"addr-width",
		"reg-width",
		"ncores",
		"num-ints",
		"timer-int",
		"syscall-int",
		"ipi-int",
		"timer-cycle-time-us",
	};


	if(type < 0 || type >= sizeof_array(names))
		return names[0];

	return names[type];
}
