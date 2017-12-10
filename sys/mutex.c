// TODO check implementation
#include <arch/atomic.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* macros */
#if defined(BUILD_KERNEL)

#include <arch/core.h>
#define LOCK_ID	(PIR)

#elif defined(BUILD_LIBSYS)

#include <lib/unistd.h>
#define LOCK_ID	({ \
	thread_info_t tinfo; \
	\
	\
	(void)thread_info(&tinfo); \
	tinfo.tid; \
})

#else
#error "invalid build flag"
#endif // BUILD_KERNEL/LIBSYS


/* global functions */
void mutex_init(mutex_t *m){
	m->nest_cnt = -1;
	m->lock = LOCK_CLEAR;
	m->lock_id = 0;
}

void mutex_init_nested(mutex_t *m){
	m->nest_cnt = 0;
	m->lock = LOCK_CLEAR;
	m->lock_id = 0;
}

void mutex_lock(mutex_t *m){
	while(1){
		if(mutex_trylock(m) == 0)
			break;

		// TODO may suspend thread
	}
}

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

int mutex_trylock(mutex_t *m){
	if(cas((int*)(&m->lock), LOCK_CLEAR, LOCK_SET) == 0){
		m->lock_id = LOCK_ID;
		return 0;
	}

	return 1;
}

int mutex_trylock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		goto_errno(err, E_INVAL);

	if(m->lock == LOCK_SET && m->lock_id == LOCK_ID){
		m->nest_cnt++;
		return 0;
	}

	return mutex_trylock(m);


err:
	return -1;
}

void mutex_unlock(mutex_t *m){
	m->lock_id = 0;
	m->lock = LOCK_CLEAR;
}

int mutex_unlock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		goto_errno(err, E_INVAL);

	if(m->nest_cnt == 0)	mutex_unlock(m);
	else					m->nest_cnt--;

	return 0;


err:
	return -1;
}
