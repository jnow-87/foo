#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H


#include <config/config.h>


/* types */
typedef enum{
	FATAL = 0x1,
	WARN = 0x2,
	DEBUG = 0x4,
	INFO = 0x8,
	STAT = 0x10
} kmsg_t;


/* macros */
#ifdef CONFIG_KERNEL_MSG_FATAL
#define	FATAL(fmt, ...)		cprintf(FATAL, "%25s()\t\033\[31m"fmt"\033\[0m", __FUNCTION__, ##__VA_ARGS__);
#else
#define FATAL(fmt, ...)
#endif

#ifdef CONFIG_KERNEL_MSG_WARN
#define WARN(fmt, ...)		cprintf(WARN, "%25s()\t\033\[33m"fmt"\033\[0m", __FUNCTION__, ##__VA_ARGS__);
#else
#define WARN(fmt, ...)
#endif

#ifdef CONFIG_KERNEL_MSG_DEBUG
#define DEBUG(fmt, ...)		cprintf(DEBUG, "[DEBUG] %25s()\t"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#ifdef CONFIG_KERNEL_MSG_INFO
#define INFO(fmt, ...)		cprintf(INFO, "[INFO] "fmt, ##__VA_ARGS__)
#else
#define INFO(fmt, ...)
#endif

#ifdef CONFIG_KERNEL_MSG_STAT
#define STAT(fmt, ...)		kprintf(STAT, fmt, ##__VA_ARGS__)
#else
#define STAT(fmt, ...)
#endif


/* protoypes */
#if CONFIG_KERNEL_MSG_FATAL || CONFIG_KERNEL_MSG_WARN || CONFIG_KERNEL_MSG_DEBUG || CONFIG_KERNEL_MSG_INFO || CONFIG_KERNEL_MSG_STAT

int kprintf(kmsg_t lvl, const char* format, ...);
int cprintf(kmsg_t lvl, const char* format, ...);

#endif


#endif // KERNEL_PRINTF_H
