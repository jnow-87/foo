#ifndef LIB_TIME_H
#define LIB_TIME_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/time.h>


/* macros */
#ifndef CONFIG_SC_TIME

#define time(t)	CALL_DISABLED(time, CONFIG_SC_TIME)

#endif // CONFIG_SC_TIME


/* prototypes */
#ifdef CONFIG_SC_TIME

int time(time_t *t);

#endif // CONFIG_SC_TIME


#endif // LIB_TIME_H
