#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H


#include <config/config.h>
#include <arch/core.h>
#include <sys/escape.h>


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

#ifdef CONFIG_KERNEL_MSG_FATAL
#define	FATAL(fmt, ...)		cprintf(KMSG_FATAL, FG_RED "[FATAL]" RESET_ATTR " %25s()\t" FG_RED fmt RESET_ATTR, __FUNCTION__, ##__VA_ARGS__);
#else
#define FATAL(fmt, ...)
#endif // CONFIG_KERNEL_MSG_FATAL

#ifdef CONFIG_KERNEL_MSG_WARN
#define WARN(fmt, ...)		cprintf(KMSG_WARN, FG_YELLOW "[WARN]" RESET_ATTR " %25s()\t" FG_YELLOW fmt RESET_ATTR, __FUNCTION__, ##__VA_ARGS__);
#else
#define WARN(fmt, ...)
#endif // CONFIG_KERNEL_MSG_WARN

#ifdef CONFIG_KERNEL_MSG_DEBUG
#define DEBUG(fmt, ...)		cprintf(KMSG_DEBUG, "[DEBUG] %25s()\t"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif // CONFIG_KERNEL_MSG_DEBUG

#ifdef CONFIG_KERNEL_MSG_INFO
#define INFO(fmt, ...)		cprintf(KMSG_INFO, "[INFO] "fmt, ##__VA_ARGS__)
#else
#define INFO(fmt, ...)
#endif // CONFIG_KERNEL_MSG_STAT

#ifdef CONFIG_KERNEL_MSG_STAT
#define STAT(fmt, ...)		kprintf(KMSG_STAT, fmt, ##__VA_ARGS__)
#else
#define STAT(fmt, ...)
#endif // CONFIG_KERNEL_MSG_STAT

#if CONFIG_KERNEL_MSG_FATAL || CONFIG_KERNEL_MSG_WARN || CONFIG_KERNEL_MSG_DEBUG || CONFIG_KERNEL_MSG_INFO || CONFIG_KERNEL_MSG_STAT
#ifdef CONFIG_KERNEL_SMP

#define cprintf(lvl, fmt, ...) \
	kprintf(lvl, "[%u] " fmt, PIR, ##__VA_ARGS__)

#else

#define cprintf(lvl, fmt, ...) \
	kprintf(lvl, fmt, ##__VA_ARGS__)

#endif // CONFIG_KERNEL_SMP
#endif // CONFIG_KERNEL_MSG_*


/* protoypes */
#if CONFIG_KERNEL_MSG_FATAL || CONFIG_KERNEL_MSG_WARN || CONFIG_KERNEL_MSG_DEBUG || CONFIG_KERNEL_MSG_INFO || CONFIG_KERNEL_MSG_STAT

void kprintf(kmsg_t lvl, char const *format, ...);

#else

#define kprintf(lvl, fmt, ...)

#endif // CONFIG_KERNEL_MSG_*


#endif // KERNEL_PRINTF_H
