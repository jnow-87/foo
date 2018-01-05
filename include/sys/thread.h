#ifndef SYS_THREAD_H
#define SYS_THREAD_H


/* types */
typedef enum{
	DESTROYED = 0,
	CREATED,
	READY,
	WAITING,
	RUNNING,
	NTHREADSTATES
} thread_state_t;


#endif // SYS_THREAD_H
