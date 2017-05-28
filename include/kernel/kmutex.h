#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H


#include <config/config.h>
#include <arch/interrupt.h>
#include <sys/compiler.h>
#include <sys/mutex.h>


/* types */
#if CONFIG_KERNEL_SMP == 1

typedef struct{
	int_type_t imask;
	mutex_t m;
} kmutex_t;

#else // CONFIG_KERNEL_SMP

typedef char kmutex_t __unused;

#endif // CONFIG_KERNEL_SMP


/* macros */
#if CONFIG_KERNEL_SMP == 1

#define _KMUTEX_INITIALISER(nest){ \
	.imask = INT_NONE, \
	.m = _MUTEX_INITIALISER(nest) \
}

#define KMUTEX_INITIALISER() 		_KMUTEX_INITIALISER(-1)
#define NESTED_KMUTEX_INITIALISER()	_KMUTEX_INITIALISER(0)

#define kmutex_init_nested(_m) 		mutex_init_nested(&((_m)->m))
#define kmutex_lock_nested(_m) 		mutex_lock_nested(&((_m)->m))
#define kmutex_unlock_nested(_m) 	mutex_unlock_nested(&((_m)->m))
#define kmutex_trylock_nested(_m) 	mutex_trylock_nested(&((_m)->m))

#define kmutex_init(_m){ \
	mutex_init(&((_m)->m)); \
	(_m)->imask = INT_NONE; \
}

#define kmutex_lock(_m){ \
	mutex_lock(&((_m)->m)); \
	(_m)->imask = int_enabled(); \
	int_enable(INT_NONE); \
}

#define kmutex_unlock(_m){ \
	mutex_unlock(&((_m)->m)); \
	int_enable((_m)->imask); \
}

#define kmutex_trylock(_m) ({ \
	int r; \
	\
	\
	if((r = mutex_trylock(&((_m)->m))) == 0){ \
		(_m)->imask = int_enabled(); \
		int_enable(INT_NONE); \
	} \
	r; \
})

#else // CONFIG_KERNEL_SMP

#define _KMUTEX_INITIALISER(nest)	0
#define KMUTEX_INITIALISER()		0
#define NESTED_KMUTEX_INITIALISER	0

#define kmutex_init_nested(_m)
#define kmutex_lock_nested(_m)
#define kmutex_unlock(_m)
#define kmutex_trylock_nested(_m)

#define kmutex_init(_m)
#define kmutex_lock(_m)
#define kmutex_unlock(_m)
#define kmutex_trylock(_m)

#endif // CONFIG_KERNEL_SMP
#endif // KERNEL_MUTEX_H
