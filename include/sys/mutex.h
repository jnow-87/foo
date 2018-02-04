#ifndef SYS_MUTEX_H
#define SYS_MUTEX_H


#include <sys/compiler.h>
#include <sys/process.h>
#include <sys/thread.h>
#include <sys/types.h>


/* macros */
#define LOCK_CLEAR	0
#define LOCK_SET	1

#define _MUTEX_INITIALISER(_attr){ \
	.attr = (_attr), \
	.nest_cnt = 0, \
	.lock = LOCK_CLEAR, \
}

#define MUTEX_INITIALISER()			_MUTEX_INITIALISER(MTX_NONE)
#define NESTED_MUTEX_INITIALISER()	_MUTEX_INITIALISER(MTX_NESTED)


/* types */
#ifdef BUILD_KERNEL

typedef struct{
	pid_t pid;
	tid_t tid;
} lock_id_t;

#else

typedef tid_t lock_id_t;

#endif // BUILD_KERNEL

typedef enum{
	MTX_NONE = 0x0,
	MTX_NESTED = 0x1,
} mutex_attr_t;

typedef struct{
	volatile int lock					// indicates if the mutex is locked
#ifdef ARCH_CACHELINE_SIZE
		__align(ARCH_CACHELINE_SIZE); 	// force alignment of lock to a cache line
#else
		;
#endif

	mutex_attr_t attr;

	uint8_t nest_cnt;
	lock_id_t lock_id;					// contains the lock id for locked, nested mutexes
										// it is undefined for non-nested and unlocked mutexes
} mutex_t;


/* prototypes */
void mutex_init(mutex_t *m, mutex_attr_t attr);
void mutex_lock(mutex_t *m);
int mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);


#endif // SYS_MUTEX_H
