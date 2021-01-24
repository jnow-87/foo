/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_LINUX_H
#define X86_LINUX_H


#include <arch/x86/opts.h>
#include <kernel/panic.h>
#include <sys/stdarg.h>
#include <sys/types.h>
#include <sys/escape.h>


/* macros */
#define lnx_printf(fmt, ...) lnx_dprintf(1, fmt, ##__VA_ARGS__)

#ifdef BUILD_KERNEL

#define DEBUG_STR	"[kernel]"
#define DEBUG_COLOR	FG_BLUE

#define LNX_SYSCALL_ERROR_EXIT(fmt, ...) \
	kpanic("error in linux syscall " FG_VIOLETT "%25.25s:%-5u " DEBUG_COLOR "%-20.20s " RESET_ATTR fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#else

#define DEBUG_STR	"[init]  "
#define DEBUG_COLOR	FG_KOBALT

#define LNX_SYSCALL_ERROR_EXIT(fmt, ...){ \
	LNX_ERROR("error in linux syscall "); \
	LNX_EEXIT(fmt "\n", ##__VA_ARGS__); \
}

#endif // BUILD_KERNEL


#define LNX_ERROR(fmt, ...) \
	lnx_dprintf(2, FG_RED DEBUG_STR " error" FG_VIOLETT " %19.19s:%-5u " DEBUG_COLOR "%-20.20s " RESET_ATTR fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#define LNX_EEXIT(fmt, ...){ \
	LNX_ERROR(fmt, ##__VA_ARGS__); \
	lnx_dprintf(2, "exit...\n"); \
	lnx_exit(1); \
}

#define LNX_DEBUG(fmt, ...){ \
	if(x86_opts.debug) \
		lnx_dprintf(2, DEBUG_COLOR DEBUG_STR FG_VIOLETT " %25.25s:%-5u " DEBUG_COLOR "%-20.20s " RESET_ATTR fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
}


/* types */
typedef void (*lnx_sig_hdlr_t)(int sig);

typedef enum{
	LNX_SYS_READ = 0,
	LNX_SYS_WRITE = 1,
	LNX_SYS_OPEN = 2,
	LNX_SYS_CLOSE = 3,
	LNX_SYS_LSEEK = 8,
	LNX_SYS_SIGACTION = 13,
	LNX_SYS_SIGRETURN_RT = 15,
	LNX_SYS_PAUSE = 34,
	LNX_SYS_NANOSLEEP = 35,
	LNX_SYS_GETPID = 39,
	LNX_SYS_EXIT = 60,
	LNX_SYS_KILL = 62,
	LNX_SYS_GETPPID = 110,
} lnx_syscall_t;


/* prototypes */
long int lnx_syscall(unsigned long int num, unsigned long int args[6], int tolerant);

long int lnx_open(char const *path, int flags, int mode);
void lnx_close(int fd);

ssize_t lnx_read(int fd, void *buf, size_t n);
void lnx_read_fix(int fd, void *buf, size_t n);
void lnx_write(int fd, void const *buf, size_t n);

long int lnx_lseek(int fd, long int offset, int whence);

void lnx_dprintf(int fd, char const *fmt, ...);
void lnx_vdprintf(int fd, char const *fmt, va_list lst);

void lnx_sigset(int sig, lnx_sig_hdlr_t hdlr);
void lnx_kill(int pid, int sig);

void lnx_pause(void);

void lnx_nanosleep(long int ns);

long int lnx_getpid(void);
long int lnx_getppid(void);

void lnx_exit(int code);


#endif // X86_LINUX_H
