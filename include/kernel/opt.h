/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_OPT_H
#define KERNEL_OPT_H


#include <config/config.h>
#include <kernel/kprintf.h>
#include <kernel/binloader.h>
#include <sys/term.h>
#include <sys/types.h>


/* macros */
// kernel message levels
#ifdef CONFIG_KERNEL_MSG_FATAL
#define CONFIG_KMSG_FATAL	KMSG_FATAL
#else
#define CONFIG_KMSG_FATAL	0x0
#endif // CONFIG_KERNEL_MSG_FATAL

#ifdef CONFIG_KERNEL_MSG_WARN
#define CONFIG_KMSG_WARN	KMSG_WARN
#else
#define CONFIG_KMSG_WARN	0x0
#endif // CONFIG_KERNEL_MSG_WARN

#ifdef CONFIG_KERNEL_MSG_INFO
#define CONFIG_KMSG_INFO	KMSG_INFO
#else
#define CONFIG_KMSG_INFO	0x0
#endif // CONFIG_KERNEL_MSG_INFO

#ifdef CONFIG_KERNEL_MSG_DEBUG
#define CONFIG_KMSG_DEBUG	KMSG_DEBUG
#else
#define CONFIG_KMSG_DEBUG	0x0
#endif // CONFIG_KERNEL_MSG_DEBUG

#ifdef CONFIG_KERNEL_STAT
#define CONFIG_KMSG_STAT	KMSG_STAT
#else
#define CONFIG_KMSG_STAT	0x0
#endif // CONFIG_KERNEL_STAT


// kernel option initializer
#define KOPT_INITIALISER(){ \
	.kernel_test = true, \
	.kernel_stat = true, \
	.init_bin = (void*)CONFIG_INIT_BINARY, \
	.init_type = CONFIG_INIT_BINTYPE, \
	.init_arg = CONFIG_INIT_ARGS, \
	.dbg_lvl = CONFIG_KMSG_FATAL | CONFIG_KMSG_WARN | CONFIG_KMSG_INFO | CONFIG_KMSG_DEBUG | CONFIG_KMSG_STAT, \
	.term_cfg = { \
		.baud = CONFIG_KERNEL_UART_BAUDRATE, \
		.stopb = CONFIG_KERNEL_UART_STOPBITS, \
		.csize = TCS_8, \
		.parity = CONFIG_KERNEL_UART_PARITY, \
	}, \
}


/* types */
typedef struct{
	/* kernel general */
	bool kernel_test,			// define wether to call kernel_test() (true - enabled, false - disabled)
		 kernel_stat;			// print kernel statistics

	void *init_bin;				// memory address to init file
	bin_type_t init_type;		// binary type of init
	char const init_arg[255];	// argument string for init

	kmsg_t dbg_lvl;				// kernel debug level

	term_cfg_t term_cfg;
} kopt_t;


/* external variables */
extern kopt_t kopt;


#endif // KERNEL_OPT_H
