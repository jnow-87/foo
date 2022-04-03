/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_LINUX_H
#define X86_LINUX_H


#include <arch/x86/opts.h>
#include <sys/stdarg.h>
#include <sys/types.h>
#include <sys/escape.h>

#ifdef BUILD_KERNEL
# include <kernel/panic.h>
#endif // BUILD_KERNEL


/* macros */
#ifdef BUILD_KERNEL
# define DEBUG_STR		"[kernel]"
# define DEBUG_COLOR	FG_BLUE

# define LNX_SYSCALL_ERROR_EXIT(fmt, ...) \
	kpanic("error in linux syscall " FG_VIOLETT "%25.25s:%-5u " DEBUG_COLOR "%-20.20s " RESET_ATTR fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#else
# define DEBUG_STR		"[init]  "
# define DEBUG_COLOR	FG_KOBALT

# define LNX_SYSCALL_ERROR_EXIT(fmt, ...){ \
	LNX_ERROR("error in linux syscall "); \
	LNX_EEXIT(fmt "\n", ##__VA_ARGS__); \
}
#endif // BUILD_KERNEL

#define lnx_printf(fmt, ...) lnx_dprintf(1, fmt, ##__VA_ARGS__)

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

typedef struct{
	uint64_t data[16];
} lnx_sigset_t;

typedef enum{
	LNX_SYS_READ = 0,
	LNX_SYS_WRITE = 1,
	LNX_SYS_OPEN = 2,
	LNX_SYS_CLOSE = 3,
	LNX_SYS_LSEEK = 8,
	LNX_SYS_MMAP = 9,
	LNX_SYS_SIGACTION = 13,
	LNX_SYS_SIGRETURN_RT = 15,
	LNX_SYS_PAUSE = 34,
	LNX_SYS_NANOSLEEP = 35,
	LNX_SYS_GETPID = 39,
	LNX_SYS_EXIT = 60,
	LNX_SYS_KILL = 62,
	LNX_SYS_FCNTL = 72,
	LNX_SYS_CHDIR = 80,
	LNX_SYS_MKDIR = 83,
	LNX_SYS_GETPPID = 110,
} lnx_syscall_t;

typedef enum{
	LNX_O_RDONLY = 0x0,
	LNX_O_WRONLY = 0x1,
	LNX_O_RDWR = 0x2,
	LNX_O_CREAT = 0x40,
	LNX_O_NONBLOCK = 0x800,
} lnx_f_mode_t;

typedef enum{
	LNX_F_SETFL = 0x4,
} lnx_fcntl_cmd_t;


/* prototypes */
long int lnx_syscall(unsigned long int num, unsigned long int args[6], int tolerant);

long int lnx_open(char const *path, int flags, int mode);
void lnx_close(int fd);

ssize_t lnx_read(int fd, void *buf, size_t n);
void lnx_read_fix(int fd, void *buf, size_t n);
void lnx_write(int fd, void const *buf, size_t n);

long int lnx_lseek(int fd, long int offset, int whence);
long int lnx_fcntl(int fd, long int cmd, long int arg);

void lnx_chdir(char const *path);
void lnx_mkdir(char const *path, int mode);

void *lnx_mmap(void *addr, size_t len, int prot, int flags, int fd, unsigned long int offset);

int lnx_dprintf(int fd, char const *fmt, ...);
int lnx_vdprintf(int fd, char const *fmt, va_list lst);

void lnx_sigaction(int sig, lnx_sig_hdlr_t hdlr, lnx_sigset_t *blocked);
void lnx_sigaddset(lnx_sigset_t *set, int sig);
void lnx_kill(int pid, int sig);

void lnx_pause(void);

void lnx_nanosleep(long int ns);

long int lnx_getpid(void);
long int lnx_getppid(void);

void lnx_exit(int code);


#endif // X86_LINUX_H
