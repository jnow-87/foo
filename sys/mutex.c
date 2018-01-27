// TODO check implementation
#include <arch/atomic.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/compiler.h>


/* macros */
#ifdef BUILD_KERNEL

#include <kernel/sched.h>


#define LOCK_ID	({ \
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

#define LOCK_ID_EQ(id0, id1)	((id0).pid == (id1).pid && (id0).tid == (id1).tid)

#else

#include <lib/unistd.h>


#define LOCK_ID	({ \
	thread_info_t _tinfo; \
	\
	\
	(void)thread_info(&_tinfo); \
	_tinfo.tid; \
})

#define LOCK_ID_EQ(id0, id1)	((id0) == (id1))

#endif // BUILD_KERNEL


/* global functions */
/**
 * \brief	initialise a mutex
 *
 * \param	m	mutex to initialise
 */
void mutex_init(mutex_t *m){
	m->nest_cnt = -1;
	m->lock = LOCK_CLEAR;
}

/**
 * \brief	initialise a nestable mutex
 *
 * \param	m	mutex to initialise
 */
void mutex_init_nested(mutex_t *m){
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
	}
}

/**
 * \brief	lock a nestable mutex block until
 * 			the lock has be acquired
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			<0	mutex is not nestable
 */
int mutex_lock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		goto_errno(err, E_INVAL);

	while(1){
		if(mutex_trylock_nested(m) == 0)
			break;

		// TODO may suspend thread
	}

	return 0;


err:
	return -1;
}

/**
 * \brief	try to lock a mutex
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			1	mutex could not be locked
 */
int mutex_trylock(mutex_t *m){
	if(cas(&m->lock, LOCK_CLEAR, LOCK_SET) == 0)
		return 0;
	return 1;
}

/**
 * \brief	try to lock a mutex
 *
 * \param	m	mutex to lock
 *
 * \return	0	mutex has been locked
 * 			1	mutex could not be locked
 * 			<0	mutex is not nestable
 */
int mutex_trylock_nested(mutex_t *m){
	lock_id_t id = LOCK_ID;


	if(m->nest_cnt == -1)
		goto_errno(err, E_INVAL);

	if(m->lock == LOCK_SET && LOCK_ID_EQ(m->lock_id, id)){
		m->nest_cnt++;
		return 0;
	}

	if(mutex_trylock(m) == 0){
		m->lock_id = id;
		return 0;
	}

	return 1;

err:
	return -1;
}

/**
 * \brief	unlock a mutex
 *
 * \param	m	mutex to unlock
 */
void mutex_unlock(mutex_t *m){
	m->lock = LOCK_CLEAR;
}

/**
 * \brief	unlock a nestable mutex
 *
 * \param	m	mutex to unlock
 *
 * \return	0	mutex has been unlocked
 * 			<0	mutex is not nestable
 */
int mutex_unlock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		goto_errno(err, E_INVAL);

	if(m->nest_cnt == 0)	mutex_unlock(m);
	else					m->nest_cnt--;

	return 0;


err:
	return -1;
}
