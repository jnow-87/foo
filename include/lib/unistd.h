#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <config/config.h>
#include <arch/thread.h>
#include <sys/compiler.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/file.h>


/* macros */
// sleep
#ifdef CONFIG_SC_TIME

#define msleep(ms)	sleep(ms, 0)
#define usleep(us)	sleep(0, us)

#else // CONFIG_SC_TIME

#define msleep(ms)		CALL_DISABLED(msleep, CONFIG_SC_TIME)
#define usleep(us)		CALL_DISABLED(usleep, CONFIG_SC_TIME)
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

// process control
#ifndef CONFIG_SC_PROCESS

#define process_create(binary, bin_type, name, args)	CALL_DISABLED(process_create, CONFIG_SC_PROCESS)
#define process_info(info)								CALL_DISABLED(process_info, CONFIG_SC_PROCESS)

#endif // CONFIG_SC_PROCESS

// thread control
#ifndef CONFIG_SC_THREAD

#define thread_create(enry, arg)	CALL_DISABLED(thread_create, CONFIG_SC_THREAD)
#define thread_info(info)			CALL_DISABLED(thread_info, CONFIG_SC_THREAD)
#define nice(inc)					CALL_DISABLED(nice, CONFIG_SC_THREAD)

#endif // CONFIG_SC_THREAD


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
#ifdef CONFIG_SC_FILESYSTEM

int open(char const *path, f_mode_t mode);
int close(int fd);

int read(int fd, void *buf, size_t n);
int write(int fd, void *buf, size_t n);

int ioctl(int fd, int cmd, void *data, size_t data_len);
int fcntl(int fd, int request, void *data, size_t data_len);

int chdir(char const *path);
int rmdir(char const *path);

#endif // CONFIG_SC_FILESYSTEM

// process control
#ifdef CONFIG_SC_PROCESS

pid_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args);
int process_info(process_info_t *info);

#endif // CONFIG_SC_PROCESS

// thread control
#ifdef CONFIG_SC_THREAD

tid_t thread_create(int (*entry)(void *), void *arg);
int thread_info(thread_info_t *info);
int nice(int inc);

#endif // CONFIG_SC_THREAD

// sleep
#ifdef CONFIG_SC_TIME

int sleep(size_t ms, size_t us);

#endif // CONFIG_SC_TIME


#endif // LIB_UNISTD_H
