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
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/escape.h>


/* local/static prototypes */
static char putc(char c, FILE *stream);


/* static variables */
static klog_itf_t *log = 0x0;


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

	log = dt_itf;

	return E_OK;
}

device_probe("kernel,log", probe);

static char putc(char c, FILE *stream){
	static char buf[CONFIG_KERNEL_LOG_SIZE] = { 0 };
	static size_t idx = 0;


	buf[idx++] = c;

	if(log){
		if(idx == CONFIG_KERNEL_LOG_SIZE)
			strncpy(buf + CONFIG_KERNEL_LOG_SIZE - 10, RESET_ATTR "\n...\n\n", 10);

		if(c == '\n' || idx >= CONFIG_KERNEL_LOG_SIZE - 10){
			log->puts(buf, idx, log->data);
			idx = 0;
		}
	}

	if(idx >= CONFIG_KERNEL_LOG_SIZE - 10)
		idx = CONFIG_KERNEL_LOG_SIZE - 1;

	return c;
}
