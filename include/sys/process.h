/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H


#ifndef BUILD_HOST

/* macros */
#define PID_MAX	((pid_t)(~0))


/* types */
typedef unsigned int pid_t;

#endif // BUILD_HOST


#endif // SYS_PROCESS_H
