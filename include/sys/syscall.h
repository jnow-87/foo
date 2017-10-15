#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H


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


#endif // SYS_SYSCALL_H
