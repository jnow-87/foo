/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H


#include <config/config.h>
#include <arch/arch.h>
#include <sys/escape.h>
#include <sys/stdarg.h>


/* types */
typedef enum{
	KMSG_FATAL = 0x1,
	KMSG_WARN = 0x2,
	KMSG_DEBUG = 0x4,
	KMSG_INFO = 0x8,
	KMSG_STAT = 0x10,
	KMSG_MULTICORE = 0x20,
} kmsg_t;


/* macros */
#define KMSG_ANY	(KMSG_FATAL | KMSG_WARN | KMSG_DEBUG | KMSG_INFO | KMSG_STAT)

// general print macros
#if (defined(CONFIG_KERNEL_LOG_FATAL) && !defined(BUILD_KERNEL_LOG_FATAL_DISABLE))
# define FATAL(fmt, ...)	kprintf(KMSG_FATAL | KMSG_MULTICORE, FG("[FTL]", RED) " %25.25s:%-5u %-20.20s    " FG(fmt, RED), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
# define FATAL(fmt, ...)	{}
#endif // CONFIG_KERNEL_LOG_FATAL

#if (defined(CONFIG_KERNEL_LOG_WARN) && !defined(BUILD_KERNEL_LOG_WARN_DISABLE))
# define WARN(fmt, ...)		kprintf(KMSG_WARN | KMSG_MULTICORE, FG("[WRN]", YELLOW) " %25.25s:%-5u %-20.20s    " FG(fmt, YELLOW), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
# define WARN(fmt, ...)		{}
#endif // CONFIG_KERNEL_LOG_WARN

#if (defined(CONFIG_KERNEL_LOG_INFO) && !defined(BUILD_KERNEL_LOG_INFO_DISABLE))
# define INFO(fmt, ...)		kprintf(KMSG_INFO | KMSG_MULTICORE, fmt, ##__VA_ARGS__)
#else
# define INFO(fmt, ...)		{}
#endif // CONFIG_KERNEL_LOG_INFO

#if (defined(CONFIG_KERNEL_STAT) && !defined(BUILD_KERNEL_STAT_DISABLE))
# define STAT(fmt, ...)		kprintf(KMSG_STAT | KMSG_MULTICORE, fmt, ##__VA_ARGS__)
#else
# define STAT(fmt, ...)		{}
#endif // CONFIG_KERNEL_STAT

// debug print macros
#if (defined(BUILD_KERNEL_LOG_DEBUG) && !defined(BUILD_KERNEL_LOG_DEBUG_DISABLE))
# define DEBUG(fmt, ...)	kprintf(KMSG_DEBUG | KMSG_MULTICORE, "[DBG] %25.25s:%-5u %-20.20s    "fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
# define DEBUG(fmt, ...)	{}
#endif


/* prototypes */
#if CONFIG_KERNEL_LOG
void kprintf(kmsg_t lvl, char const *format, ...);
void kvprintf(kmsg_t lvl, char const *format, va_list lst);
#else
# define kprintf(lvl, fmt, ...)		{}
# define kvprintf(lvl, fmt, ...)	{}
#endif // CONFIG_KERNEL_LOG


#endif // KERNEL_PRINTF_H
