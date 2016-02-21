#ifndef SYS_TIME_H
#define SYS_TIME_H


/* macros */
#define TIME_INITIALISER \
	{ \
		.s = 0, \
		.ms = 0, \
		.us = 0 \
	}


/* types */
typedef struct{
	int s,
		ms,
		us;
} time_t;


#endif // SYS_TIME_H
