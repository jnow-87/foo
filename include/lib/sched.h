#ifndef LIB_SCHED_H
#define LIB_SCHED_H


#include <config/config.h>
#include <sys/compiler.h>


/* macros */
#ifndef CONFIG_SC_SCHED

#define sched_yield()	CALL_DISABLED(sched_yield, CONFIG_SC_SCHED)

#endif // CONFIG_SC_SCHED


/* prototypes */
#ifdef CONFIG_SC_SCHED

void sched_yield(void);

#endif // CONFIG_SC_SCHED


#endif // LIB_SCHED_H
