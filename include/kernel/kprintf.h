/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H


#include <config/config.h>
#include <arch/core.h>
#include <sys/escape.h>
#include <sys/stdarg.h>


/* types */
typedef enum{
	KMSG_FATAL = 0x1,
	KMSG_WARN = 0x2,
	KMSG_DEBUG = 0x4,
	KMSG_INFO = 0x8,
	KMSG_STAT = 0x10
} kmsg_t;


/* macros */
#define KMSG_ANY	(KMSG_FATAL | KMSG_WARN | KMSG_DEBUG | KMSG_INFO | KMSG_STAT)

// general print macros
#ifdef CONFIG_KERNEL_MSG_FATAL
#define	FATAL(fmt, ...)		cprintf(KMSG_FATAL, FG_RED "[FTL]" RESET_ATTR " %25.25s:%-20.20s    " FG_RED fmt RESET_ATTR, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define FATAL(fmt, ...)		{}
#endif // CONFIG_KERNEL_MSG_FATAL

#if !defined(CONFIG_KERNEL_MSG_WARN) \
 || (defined(BUILD_KERNEL_INIT) && !defined(CONFIG_KERNEL_EARLY_PRINT))
#define WARN(fmt, ...)		{}
#else
#define WARN(fmt, ...)		cprintf(KMSG_WARN, FG_YELLOW "[WRN]" RESET_ATTR " %25.25s:%-20.20s    " FG_YELLOW fmt RESET_ATTR, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#endif // CONFIG_KERNEL_MSG_WARN

#ifdef CONFIG_KERNEL_MSG_INFO
#define INFO(fmt, ...)		cprintf(KMSG_INFO, fmt, ##__VA_ARGS__)
#else
#define INFO(fmt, ...)		{}
#endif // CONFIG_KERNEL_MSG_INFO

#ifdef CONFIG_KERNEL_STAT
#define STAT(fmt, ...)		kprintf(KMSG_STAT, fmt, ##__VA_ARGS__)
#else
#define STAT(fmt, ...)		{}
#endif // CONFIG_KERNEL_STAT

// debug print macros
#if !defined(CONFIG_KERNEL_MSG_DEBUG) \
 || (defined(BUILD_KERNEL_INIT) && !defined(CONFIG_KERNEL_EARLY_PRINT)) \
 || (defined(BUILD_KERNEL_SYSCALL) && !defined(CONFIG_KERNEL_SC_DEBUG)) \
 || (defined(BUILD_KERNEL_FS) && !defined(CONFIG_KERNEL_FS_DEBUG)) \
 || (defined(BUILD_DRIVER) && !defined(CONFIG_DRIVER_DEBUG))
#define DEBUG(fmt, ...)		{}
#else
#define DEBUG(fmt, ...)		cprintf(KMSG_DEBUG, "[DBG] %25.25s:%-20.20s    "fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#endif

// kprintf
#if CONFIG_KERNEL_PRINTF
#ifdef CONFIG_KERNEL_SMP
#define cprintf(lvl, fmt, ...)	kprintf(lvl, "[%u] " fmt, PIR, ##__VA_ARGS__)
#else
#define cprintf(lvl, fmt, ...) 	kprintf(lvl, fmt, ##__VA_ARGS__)
#endif // CONFIG_KERNEL_SMP
#endif // CONFIG_KERNEL_PRINTF


/* prototypes */
#if CONFIG_KERNEL_PRINTF

void kprintf(kmsg_t lvl, char const *format, ...);
void kvprintf(kmsg_t lvl, char const *format, va_list lst);

#else

#define kprintf(lvl, fmt, ...)	{}
#define kvprintf(lvl, fmt, ...)	{}

#endif // CONFIG_KERNEL_PRINTF


#endif // KERNEL_PRINTF_H
