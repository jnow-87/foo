/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



// TODO check implementation
#include <arch/atomic.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/compiler.h>

#ifdef BUILD_KERNEL
# include <kernel/sched.h>
#else
# include <lib/unistd.h>
#endif // BUILD_KERNEL


/* macros */
#ifdef BUILD_KERNEL
# define LOCK_ID_EQ(id0, id1)	((id0).pid == (id1).pid && (id0).tid == (id1).tid)
# define LOCK_ID ({ \
	thread_t const *_this_t; \
	lock_id_t _id; \
	\
	\
	_this_t = sched_running(); \
	_id.pid = _this_t->parent->pid; \
	_id.tid = _this_t->tid; \
	\
	_id; \
})
#else
# define LOCK_ID_EQ(id0, id1)	((id0) == (id1))
# define LOCK_ID ({ \
	thread_info_t _tinfo; \
	\
	\
	(void)thread_info(&_tinfo); \
	_tinfo.tid; \
})
#endif // BUILD_KERNEL


/* global functions */
/**
 * \brief	initialise a mutex
 *
 * \param	m	mutex to initialise
 */
void mutex_init(mutex_t *m, mutex_attr_t attr){
	m->attr = attr;
	m->nest_cnt = 0;
	m->lock = LOCK_CLEAR;
}

/**
 * \brief	lock a mutex and will block until
 * 			the lock has been acquired
 *
 * \param	m	mutex to lock
 */
void mutex_lock(mutex_t *m){
	while(1){
		if(mutex_trylock(m) == 0)
			break;

		// TODO may suspend thread
		// 		check if this is save
		// 		if used in ISRs
	}
}

/**
 * \brief	try to lock a mutex
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			-1	mutex could not be locked
 * 			-2	nest count limit reached
 */
int mutex_trylock(mutex_t *m){
	lock_id_t id = { 0 };


	if(m->attr & MTX_NESTED){
		id = LOCK_ID;

		if(m->lock == LOCK_SET && LOCK_ID_EQ(m->lock_id, id)){
			m->nest_cnt++;

			// overflow detection
			if(m->nest_cnt == 0)
				return -2;

			return 0;
		}
	}

	if(cas(&m->lock, LOCK_CLEAR, LOCK_SET) == 0){
		if(m->attr & MTX_NESTED)
			m->lock_id = id;

		return 0;
	}

	return -1;
}

/**
 * \brief	unlock a mutex
 *
 * \param	m	mutex to unlock
 */
void mutex_unlock(mutex_t *m){
	if(m->nest_cnt == 0)	m->lock = LOCK_CLEAR;
	else					m->nest_cnt--;
}
