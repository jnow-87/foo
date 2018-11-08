/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <config/config.h>
#include <arch/thread.h>
#include <sys/compiler.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>


/* macros */
// sleep
#define msleep(ms)	sleep(ms, 0)
#define usleep(us)	sleep(0, us)


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

int read(int fd, void *buf, size_t n);
int write(int fd, void *buf, size_t n);

int ioctl(int fd, int cmd, void *data, size_t data_len);
int fcntl(int fd, int request, void *data, size_t data_len);

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

#undef msleep
#define msleep(ms)		CALL_DISABLED(msleep, CONFIG_SC_TIME)
#undef usleep
#define usleep(us)		CALL_DISABLED(usleep, CONFIG_SC_TIME)
#undef sleep
#define sleep(ms, us)	CALL_DISABLED(sleep, CONFIG_SC_TIME)

#endif // CONFIG_SC_TIME

// file system
#ifndef CONFIG_SC_FILESYSTEM

#define open(path, mode)					CALL_DISABLED(open, CONFIG_SC_FILESYSTEM)
#define close(fd)							CALL_DISABLED(close, CONFIG_SC_FILESYSTEM)
#define read(fd, buf, n)					CALL_DISABLED(read, CONFIG_SC_FILESYSTEM)
#define write(fd, buf, n)					CALL_DISABLED(write, CONFIG_SC_FILESYSTEM)
#define ioctl(fd, cmd, data, data_len)		CALL_DISABLED(ioctl, CONFIG_SC_FILESYSTEM)
#define fcntl(fd, request, data, data_len)	CALL_DISABLED(fcntl, CONFIG_SC_FILESYSTEM)
#define chdir(path)							CALL_DISABLED(chdir, CONFIG_SC_FILESYSTEM)
#define rmdir(path)							CALL_DISABLED(rmdir, CONFIG_SC_FILESYSTEM)

#endif // CONFIG_SC_FILESYSTEM

// process and thread control
#ifndef CONFIG_SC_SCHED

#define process_create(binary, bin_type, name, args)	CALL_DISABLED(process_create, CONFIG_SC_SCHED)
#define process_info(info)								CALL_DISABLED(process_info, CONFIG_SC_SCHED)

#define thread_create(enry, arg)	CALL_DISABLED(thread_create, CONFIG_SC_SCHED)
#define thread_info(info)			CALL_DISABLED(thread_info, CONFIG_SC_SCHED)
#define nice(inc)					CALL_DISABLED(nice, CONFIG_SC_SCHED)

#endif // CONFIG_SC_SCHED


#endif // LIB_UNISTD_H
