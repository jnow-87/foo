#ifndef SYS_MUTEX_H
#define SYS_MUTEX_H


#include <arch/arch.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* macros */
#define LOCK_CLEAR	0
#define LOCK_SET	1

#define _MUTEX_INITIALISER(nest){ \
	.nest_cnt = nest, \
	.lock = LOCK_CLEAR, \
	.lock_id = 0, \
}

#define MUTEX_INITIALISER()			_MUTEX_INITIALISER(-1)
#define NESTED_MUTEX_INITIALISER()	_MUTEX_INITIALISER(0)


/* types */
typedef struct{
	int nest_cnt;						// -1 indicates none-nesting mutex
	volatile uint8_t lock;
	thread_id_t lock_id;
#ifdef ARCH_CACHELINE_SIZE
		__align(ARCH_CACHELINE_SIZE); 	// force alignment of lock to a cache line
#else
		;
#endif
} mutex_t;


/* prototypes */
/**
 * \brief	initialise a mutex
 *
 * \param	m	mutex to initialise
 */
void mutex_init(mutex_t *m);

/**
 * \brief	initialise a nestable mutex
 *
 * \param	m	mutex to initialise
 */
void mutex_init_nested(mutex_t *m);

/**
 * \brief	lock a mutex and will block until
 * 			the lock has been acquired
 *
 * \param	m	mutex to lock
 */
void mutex_lock(mutex_t *m);

/**
 * \brief	lock a nestable mutex block until
 * 			the lock has be acquired
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			<0	mutex is not nestable
 */
int mutex_lock_nested(mutex_t *m);

/**
 * \brief	unlock a mutex
 *
 * \param	m	mutex to unlock
 */
void mutex_unlock(mutex_t *m);

/**
 * \brief	unlock a nestable mutex
 *
 * \param	m	mutex to unlock
 *
 * \return	0	mutex has been unlocked
 * 			<0	mutex is not nestable
 */
int mutex_unlock_nested(mutex_t *m);

/**
 * \brief	try to lock a mutex
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			1	mutex could not be locked
 */
int mutex_trylock(mutex_t *m);

/**
 * \brief	try to lock a mutex
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			1	mutex could not be locked
 * 			<0	mutex is not nestable
 */
int mutex_trylock_nested(mutex_t *m);


#endif // SYS_MUTEX_H
