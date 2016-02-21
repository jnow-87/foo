#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H


/* types */
typedef enum __packed{
	FATAL = 0x1,
	WARN = 0x2,
	DEBUG = 0x4,
	INFO = 0x8,
	STAT = 0x10
} kmsg_t;


/* macros */
#define	FATAL(fmt, ...)		cprintf(FATAL, "%25s()\t\033\[31m"fmt"\033\[0m", __FUNCTION__, ##__VA_ARGS__);
#define WARN(fmt, ...)		cprintf(WARN, "%25s()\t\033\[33m"fmt"\033\[0m", __FUNCTION__, ##__VA_ARGS__);
#define DEBUG(fmt, ...)		cprintf(DEBUG, "[DEBUG] %25s()\t"fmt, __FUNCTION__, ##__VA_ARGS__)
#define INFO(fmt, ...)		cprintf(INFO, "[INFO] "fmt, ##__VA_ARGS__)
#define STAT(fmt, ...)		kprintf(STAT, fmt, ##__VA_ARGS__)


/* protoypes */
int kprintf(kmsg_t lvl, const char* format, ...);
int cprintf(kmsg_t lvl, const char* format, ...);


#endif // KERNEL_PRINTF_H
