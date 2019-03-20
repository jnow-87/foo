/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H

#ifndef _x86_
#ifndef __x86_64__


/* macros */
#define PID_MAX	((pid_t)(~0))


/* types */
typedef unsigned int pid_t;


#endif // __x86_64__
#endif // _x86_

#endif // SYS_PROCESS_H