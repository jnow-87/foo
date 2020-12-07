/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/driver.h>
#include <kernel/opt.h>
#include <driver/klog.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/escape.h>


/* macros */
#define OVERFLOW_MSG	RESET_ATTR FG_KOBALT "\n[kernel log overflow]\n\n" RESET_ATTR


/* types */
typedef struct{
	klog_itf_t *dev;

	char buf[CONFIG_KERNEL_LOG_SIZE];
	size_t idx;
	bool overflow;
} log_t;


/* local/static prototypes */
static char putc(char c, FILE *stream);
static void flush(void);


/* static variables */
log_t log = { 0 };


/* global functions */
void kprintf(kmsg_t lvl, char const *format, ...){
	va_list lst;


	va_start(lst, format);
	kvprintf(lvl, format, lst);
	va_end(lst);
}

void kvprintf(kmsg_t lvl, char const *format, va_list lst){
	FILE fp = FILE_INITIALISER(0x0, 0x0, 0, putc);


	if((kopt.dbg_lvl & lvl) == 0)
		return;

	vfprintf(&fp, format, lst);
}


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	if(((klog_itf_t*)dt_itf)->puts == 0x0)
		return_errno(E_INVAL);

	log.dev = dt_itf;

	flush();

	return E_OK;
}

device_probe("kernel,log", probe);

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
	log.dev->puts(log.buf, log.idx, log.dev->data);

	if(log.overflow)
		log.dev->puts(OVERFLOW_MSG, strlen(OVERFLOW_MSG), log.dev->data);

	log.overflow = false;
	log.idx = 0;
}
