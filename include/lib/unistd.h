/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/process.h>
#include <sys/thread.h>
#include <sys/fcntl.h>
#include <sys/stat.h>


/* macros */
// sleep
#define msleep(ms)	sleep(ms, 0)
#define usleep(us)	sleep(0, us)

// ioctl
#define ioctl(fd, cmd, arg)	ionctl(fd, cmd, arg, sizeof(*arg))


/* types */
typedef struct{
	pid_t pid;
} process_info_t;

typedef struct{
	tid_t tid;

	unsigned int priority,
				 affinity;
} thread_info_t;


/* prototypes */
// file system
int open(char const *path, f_mode_t mode);
int dup(int old_fd);
int dup2(int old_fd, int new_fd);
int close(int fd);

ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, void const *buf, size_t n);

int ionctl(int fd, int cmd, void *arg, size_t arg_len);
int fcntl(int fd, int request, void *arg, size_t arg_len);

void *mmap(int fd, size_t n);
void munmap(void *addr);

int unlink(char const *path);

int mkdir(char const *path);
int chdir(char const *path);
int rmdir(char const *path);

int stat(char const *path, stat_t *stat);
int statat(char const *dir, char const *path, stat_t *stat);
int fstat(int fd, stat_t *stat);

// process control
pid_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args);
int process_info(process_info_t *info);

// thread control
tid_t thread_create(int (*entry)(void *), void *arg);
int thread_info(thread_info_t *info);
int nice(int inc);

// sleep
int sleep(size_t ms, size_t us);


/* disabled-call macros */
// sleep
#ifndef CONFIG_SC_TIME
# undef msleep
# define msleep(ms)							CALL_DISABLED(msleep, CONFIG_SC_TIME)

# undef usleep
# define usleep(us)							CALL_DISABLED(usleep, CONFIG_SC_TIME)

# undef sleep
# define sleep(ms, us)						CALL_DISABLED(sleep, CONFIG_SC_TIME)
#endif // CONFIG_SC_TIME

// file system
#ifndef CONFIG_SC_FILESYSTEM
# define open(path, mode)					CALL_DISABLED(open, CONFIG_SC_FILESYSTEM)
# define close(fd)							CALL_DISABLED(close, CONFIG_SC_FILESYSTEM)
# define read(fd, buf, n)					CALL_DISABLED(read, CONFIG_SC_FILESYSTEM)
# define write(fd, buf, n)					CALL_DISABLED(write, CONFIG_SC_FILESYSTEM)
# define ionctl(fd, cmd, arg, arg_len)		CALL_DISABLED(ionctl, CONFIG_SC_FILESYSTEM)
# define fcntl(fd, request, arg, arg_len)	CALL_DISABLED(fcntl, CONFIG_SC_FILESYSTEM)
# define chdir(path)						CALL_DISABLED(chdir, CONFIG_SC_FILESYSTEM)
# define rmdir(path)						CALL_DISABLED(rmdir, CONFIG_SC_FILESYSTEM)
#endif // CONFIG_SC_FILESYSTEM

// process and thread control
#ifndef CONFIG_SC_SCHED
# define process_create(binary, bin_type, name, args) \
											CALL_DISABLED(process_create, CONFIG_SC_SCHED)
# define process_info(info)					CALL_DISABLED(process_info, CONFIG_SC_SCHED)

# define thread_create(enry, arg)			CALL_DISABLED(thread_create, CONFIG_SC_SCHED)
# define thread_info(info)					CALL_DISABLED(thread_info, CONFIG_SC_SCHED)
# define nice(inc)							CALL_DISABLED(nice, CONFIG_SC_SCHED)
#endif // CONFIG_SC_SCHED


#endif // LIB_UNISTD_H
