/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_SIGNAL_H
#define SYS_SIGNAL_H


/* types */
typedef enum{
	SIG_INT = 2,
	SIG_KILL = 9,
	SIG_TERM = 15,
	SIG_USR0 = 16,
	SIG_USR1,
	SIG_USR2,
	SIG_USR3,
	SIG_MAX,
} signal_t;


#endif // SYS_SIGNAL_H
