#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <arch/thread.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/file.h>


/* macros */
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
int close(int fd);

int read(int fd, void *buf, size_t n);
int write(int fd, void *buf, size_t n);

int ioctl(int fd, int cmd, void *data, size_t data_len);
int fcntl(int fd, int request, void *data, size_t data_len);

int chdir(char const *path);
int rmdir(char const *path);

// thread control
tid_t thread_create(int (*entry)(void *), void *arg);
int thread_info(thread_info_t *info);
int nice(int inc);

// process control
pid_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args);
int process_info(process_info_t *info);

// sleep
int sleep(size_t ms, size_t us);


#endif // LIB_UNISTD_H
