#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H


#include <arch/thread.h>
#include <sys/binloader.h>
#include <sys/types.h>
#include <sys/file.h>


/* types */
// syscalls
typedef enum{
	SC_OPEN,
	SC_CLOSE,
	SC_READ,
	SC_WRITE,
	SC_IOCTL,
	SC_FCNTL,
	SC_RMNODE,
	SC_CHDIR,
	SC_MALLOC,
	SC_FREE,
	SC_EXIT,
	SC_THREADCREATE,
	SC_THREADINFO,
	SC_NICE,
	SC_PROCCREATE,
	SC_PROCINFO,
	SC_SCHEDYIELD,
	SC_SLEEP,
	NSYSCALLS
} sc_t;

// syscall arguments
typedef struct{
	void *p;
	size_t size;

	int errno;
} sc_malloc_t;

typedef struct{
	int fd;

	void *data;
	size_t data_len;

	int cmd;
	f_mode_t mode;

	int errno;
} sc_fs_t;

typedef struct{
	thread_id_t tid;

	int (*entry)(void *);
	void *arg;

	unsigned int affinity;
	int priority;

	int errno;
} sc_thread_t;

typedef struct{
	process_id_t pid;

	void *binary;
	bin_type_t bin_type;

	char const *name,
			   *args;
	size_t name_len,
		   args_len;

	int errno;
} sc_process_t;

typedef struct{
	size_t us,
		   ms;

	int errno;
} sc_sleep_t;

#endif // SYS_SYSCALL_H
