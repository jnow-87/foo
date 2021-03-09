/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H


/* macros */
#ifndef BUILD_HOST
# define PID_MAX	((pid_t)(~0))
#endif // BUILD_HOST

/* types */
#ifndef BUILD_HOST
typedef unsigned int pid_t;
#endif // BUILD_HOST


#endif // SYS_PROCESS_H
