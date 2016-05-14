#ifndef SYS_MUTEX_H
#define SYS_MUTEX_H


#include <arch/arch.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* macros */
#define _MUTEX_INITIALISER(nest){ \
	.nest_cnt = nest, \
	.lock.lock_c[3] = 0, \
	.lock.lock_c[2] = 0xff, \
	.waiting = 0 \
}

#define MUTEX_INITIALISER()			_MUTEX_INITIALISER(-1)
#define NESTED_MUTEX_INITIALISER()	_MUTEX_INITIALISER(0)


/* types */
typedef union{
	volatile char lock_c[4];			// lock[3] indicates lock status (0 = free, 1 = locked)
										// lock[2] indicates lock id (core number - 0xff = none)
	volatile unsigned int lock_i;
} mutex_lock_t;

typedef struct{
	int nest_cnt;						// -1 indicates none-nesting mutex
	unsigned int waiting;				// bitfield for all cores that tryed to lock
	volatile mutex_lock_t lock
#ifdef ARCH_CACHELINE_SIZE
		__align(ARCH_CACHELINE_SIZE); 	// force alignment of lock to a cache line
#else
		;
#endif
} mutex_t;


/* prototypes */
void mutex_init(mutex_t *m);
void mutex_init_nested(mutex_t *m);

void mutex_lock(mutex_t *m);
int mutex_lock_nested(mutex_t *m);

void mutex_unlock(mutex_t *m);
void mutex_unlock_nested(mutex_t *m);

int mutex_trylock(mutex_t *m);
int mutex_trylock_nested(mutex_t *m);


#endif // SYS_MUTEX_H
