/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_MUTEX_H
#define SYS_MUTEX_H


#include <config/config.h>
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

#define LOCK_SECTION(mtx, expr){ \
	mutex_lock(mtx); \
	expr; \
	mutex_unlock(mtx); \
}


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
	int volatile lock					// indicates if the mutex is locked
#ifdef ARCH_CACHELINE_SIZE
		__align(ARCH_CACHELINE_SIZE) 	// force alignment of lock to a cache line
#endif
		;

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


/* disabled-call macros */
#if defined(BUILD_LIBSYS) && !defined(CONFIG_LIB_MUTEX)
# define mutex_init(m, attr)	CALL_DISABLED(mutex_init, CONFIG_LIB_MUTEX)
# define mutex_lock(m)			CALL_DISABLED(mutex_lock, CONFIG_LIB_MUTEX)
# define mutex_trylock(m)		CALL_DISABLED(mutex_trylock, CONFIG_LIB_MUTEX)
# define mutex_unlock(m)		CALL_DISABLED(mutex_unlock, CONFIG_LIB_MUTEX)
#endif // CONFIG_LIB_MUTEX


#endif // SYS_MUTEX_H
