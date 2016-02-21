#ifndef KERNEL_OPT_H
#define KERNEL_OPT_H


#include <config/config.h>
#include <kernel/kprintf.h>
#include <sys/types.h>


/* macros */
// kernel message levels
#ifdef CONFIG_KERNEL_MSG_FATAL
#define KMSG_FATAL	FATAL
#else
#define KMSG_FATAL	0x0
#endif // CONFIG_KERNEL_MSG_FATAL

#ifdef CONFIG_KERNEL_MSG_WARN
#define KMSG_WARN	WARN
#else
#define KMSG_WARN	0x0
#endif // CONFIG_KERNEL_MSG_WARN

#ifdef CONFIG_KERNEL_MSG_INFO
#define KMSG_INFO	INFO
#else
#define KMSG_INFO	0x0
#endif // CONFIG_KERNEL_MSG_INFO

#ifdef CONFIG_KERNEL_MSG_DEBUG
#define KMSG_DEBUG	DEBUG
#else
#define KMSG_DEBUG	0x0
#endif // CONFIG_KERNEL_MSG_DEBUG

#ifdef CONFIG_KERNEL_MSG_STAT
#define KMSG_STAT	STAT
#else
#define KMSG_STAT	0x0
#endif // CONFIG_KERNEL_MSG_STAT

// kernel option initializer
#define KOPT_INITIALISER { \
	.kernel_test = false, \
	.kernel_stat = true, \
	.init_elf = (void*)RAMFS_BASE, \
	.init_arg = "init", \
	.dbg_lvl = KMSG_FATAL | KMSG_WARN | KMSG_INFO | KMSG_DEBUG | KMSG_STAT, \
}


/* types */
typedef struct{
	/* kernel general */
	bool kernel_test,			// define wether to call kernel_test() (true - enabled, false - disabled)
		 kernel_stat;			// print kernel statistics

	void* init_elf;				// memory address to init elf file
	const char init_arg[255];	// argument string for init

	kmsg_t dbg_lvl;				// kernel debug level
} kopt_t;


/* external variables */
extern kopt_t kopt;


#endif // KERNEL_OPT_H
