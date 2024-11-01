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

#ifdef BUILD_KERNEL
# include <kernel/interrupt.h>
#endif // BUILD_KERNEL


/* macros */
#define LOCK_CLEAR	0
#define LOCK_SET	1

#ifdef BUILD_KERNEL
# define _MUTEX_INITIALISER(_attr) (mutex_t){ \
	.attr = _attr, \
	.nest_cnt = 0, \
	.lock = LOCK_CLEAR, \
	.int_en = false, \
}

# define NOINT_MUTEX_INITIALISER() _MUTEX_INITIALISER(MTX_NOINT)
#else
# define _MUTEX_INITIALISER(_attr) (mutex_t){ \
	.attr = _attr, \
	.nest_cnt = 0, \
	.lock = LOCK_CLEAR, \
}
#endif // BUILD_KERNEL

#define MUTEX_INITIALISER()			_MUTEX_INITIALISER(MTX_NONE)
#define NESTED_MUTEX_INITIALISER()	_MUTEX_INITIALISER(MTX_NESTED)

// mutex lock alignment to a cache line
#ifdef ARCH_CACHELINE_SIZE
# define LOCK_ALIGN	__align(ARCH_CACHELINE_SIZE)
#else
# define LOCK_ALIGN
#endif // ARCH_CACHELINE_SIZE

#define MUTEXED(mtx, expr){ \
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
#ifdef BUILD_KERNEL
	MTX_NOINT = 0x2,
#endif // BUILD_KERNEL
} mutex_attr_t;

typedef struct{
	int volatile lock LOCK_ALIGN;	// indicates if the mutex is locked

	mutex_attr_t attr;

	uint8_t nest_cnt;
	lock_id_t lock_id;				// contains the lock id for locked, nested mutexes
									// it is undefined for non-nested and unlocked mutexes

#ifdef BUILD_KERNEL
	bool int_en;
#endif // BUILD_KERNEL
} mutex_t;


/* prototypes */
void mutex_init(mutex_t *m, mutex_attr_t attr);
void mutex_lock(mutex_t *m);
int mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);


/* disabled-call macros */
#if defined(BUILD_LIBBRICK) && !defined(CONFIG_LIB_MUTEX)
# define mutex_init(m, attr)	CALL_DISABLED(mutex_init, CONFIG_LIB_MUTEX)
# define mutex_lock(m)			CALL_DISABLED(mutex_lock, CONFIG_LIB_MUTEX)
# define mutex_trylock(m)		CALL_DISABLED(mutex_trylock, CONFIG_LIB_MUTEX)
# define mutex_unlock(m)		CALL_DISABLED(mutex_unlock, CONFIG_LIB_MUTEX)
#endif // CONFIG_LIB_MUTEX


#endif // SYS_MUTEX_H
