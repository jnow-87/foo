// TODO check implementation
#include <config/config.h>
#include <arch/atomic.h>
#include <arch/core.h>
#include <sys/mutex.h>


/* macros */
#define LOCK_CLEAR		0xff00		// lock_id = 0xff, lock = 0
#define LOCK_SET(pir)	(((pir) & 0xff) << 8 | 0x1)


/* static variables */
#ifdef CONFIG_SYS_MUTEX_DEBUG
mutex_t *mutex_debug_try[8] = { 0, 0, 0, 0, 0, 0, 0, 0 },
		*mutex_debug_hold[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif // CONFIG_SYS_MUTEX_DEBUG


/* global functions */
void mutex_init(mutex_t *m){
	m->nest_cnt = -1;
	m->lock.lock_i = LOCK_CLEAR;
	m->waiting = 0;
}

void mutex_init_nested(mutex_t *m){
	m->nest_cnt = 0;
	m->lock.lock_i = LOCK_CLEAR;
	m->waiting = 0;
}

void mutex_lock(mutex_t *m){
#ifdef CONFIG_SYS_MUTEX_DEBUG
	mutex_debug_try[PIR] = m;
#endif // CONFIG_SYS_MUTEX_DEBUG


	while(1){
		if(mutex_trylock(m) == 0){
#ifdef CONFIG_SYS_MUTEX_DEBUG
			mutex_debug_hold[PIR] = m;
			mutex_debug_try[PIR] = 0;
#endif // CONFIG_SYS_MUTEX_DEBUG
			m->waiting &= !(m->waiting & (1 << PIR));

			break;
		}

		m->waiting |= (1 << PIR);
	}
}

int mutex_lock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		return -1;

#ifdef CONFIG_SYS_MUTEX_DEBUG
	mutex_debug_try[PIR] = m;
#endif // CONFIG_SYS_MUTEX_DEBUG

	while(1){
		if(mutex_trylock_nested(m) == 0){
			m->waiting &= !(m->waiting & (1 << PIR));

#ifdef CONFIG_SYS_MUTEX_DEBUG
			mutex_debug_hold[PIR] = m;
			mutex_debug_try[PIR] = 0;
#endif // CONFIG_SYS_MUTEX_DEBUG

			break;
		}

		m->waiting |= (1 << PIR);
	}

	return 0;
}

int mutex_trylock(mutex_t *m){
	return cas((int*)(&m->lock.lock_i), LOCK_CLEAR, LOCK_SET(PIR));
}

int mutex_trylock_nested(mutex_t *m){
	if(m->lock.lock_i == LOCK_SET(PIR)){
		m->nest_cnt++;
		return 0;
	}
	else if(m->lock.lock_i == LOCK_CLEAR)
		return cas((int*)(&m->lock.lock_i), LOCK_CLEAR, LOCK_SET(PIR));

	return -1;
}

void mutex_unlock(mutex_t *m){
#ifdef CONFIG_SYS_MUTEX_DEBUG
	mutex_debug_hold[PIR] = 0;
#endif // CONFIG_SYS_MUTEX_DEBUG

	m->lock.lock_i = LOCK_CLEAR;
}

void mutex_unlock_nested(mutex_t *m){
	if(m->nest_cnt == -1)
		return;

	if(m->nest_cnt == 0)
		mutex_unlock(m);
	else
		m->nest_cnt--;
}
