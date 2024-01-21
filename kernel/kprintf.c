/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/driver.h>
#include <kernel/opt.h>
#include <driver/klog.h>
#include <sys/devicetree.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/escape.h>
#include <sys/mutex.h>


/* macros */
#define OVERFLOW_MSG	RESET_ATTR FG("\n[kernel log overflow]", KOBALT) "\n\n"


/* types */
typedef struct{
	klog_itf_t *dev;

	char buf[CONFIG_KERNEL_LOG_SIZE];
	size_t idx;
	bool overflow;

	mutex_t mtx;
} log_t;


/* local/static prototypes */
static char putc(char c, FILE *stream);
static void flush(void);


/* static variables */
log_t log = {
	.dev = 0x0,
	.buf = { 0 },
	.idx = 0,
	.overflow = false,
	.mtx = NOINT_MUTEX_INITIALISER(),
};


/* global functions */
void kprintf(kmsg_t lvl, char const *format, ...){
	va_list lst;


	va_start(lst, format);
	kvprintf(lvl, format, lst);
	va_end(lst);
}

void kvprintf(kmsg_t lvl, char const *format, va_list lst){
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, putc);
#ifdef DEVTREE_ARCH_MULTI_CORE
	char pir[8];
#endif // DEVTREE_ARCH_MULTI_CORE


	if((kopt.dbg_lvl & lvl) == 0)
		return;

	mutex_lock(&log.mtx);

#ifdef DEVTREE_ARCH_MULTI_CORE
	if(lvl & KMSG_MULTICORE){
		snprintf(pir, 8, "[%u] ", PIR);
		vfprintf(&fp, pir, lst);
	}
#endif // DEVTREE_ARCH_MULTI_CORE

	vfprintf(&fp, format, lst);

	mutex_unlock(&log.mtx);
}


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	mutex_lock(&log.mtx);

	if(((klog_itf_t*)dt_itf)->puts != 0x0){
		log.dev = dt_itf;
		flush();
	}
	else
		set_errno(E_INVAL);

	mutex_unlock(&log.mtx);

	return 0x0;
}

driver_probe("kernel,log", probe);

static char putc(char c, FILE *stream){
	if(!log.overflow)
		log.buf[log.idx++] = c;

	if(log.dev){
		if(c == '\n' || log.idx >= CONFIG_KERNEL_LOG_SIZE)
			flush();
	}

	if(log.idx >= CONFIG_KERNEL_LOG_SIZE)
		log.overflow = true;

	return c;
}

static void flush(void){
	log.dev->puts(log.buf, log.idx, log.dev->hw);

	if(log.overflow)
		log.dev->puts(OVERFLOW_MSG, strlen(OVERFLOW_MSG), log.dev->hw);

	log.overflow = false;
	log.idx = 0;
}
