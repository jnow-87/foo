#ifndef SYS_THREAD_H
#define SYS_THREAD_H


/* macros */
#define TID_MAX	((tid_t)(~0))


/* types */
typedef enum{
	DEAD = 0,
	CREATED,
	READY,
	WAITING,
	RUNNING,
	NTHREADSTATES
} thread_state_t;

typedef unsigned int tid_t;


#endif // SYS_THREAD_H
