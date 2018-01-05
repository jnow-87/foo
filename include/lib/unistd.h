#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <arch/thread.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/file.h>


/* types */
typedef struct{
	process_id_t pid;
} process_info_t;

typedef struct{
	thread_id_t tid;

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
thread_id_t thread_create(int (*entry)(void *), void *arg);
int thread_info(thread_info_t *info);
int nice(int inc);

// process control
process_id_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args);
int process_info(process_info_t *info);


#endif // LIB_UNISTD_H
