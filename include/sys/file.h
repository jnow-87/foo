#ifndef SYS_FILE_H
#define SYS_FILE_H


#include <sys/errno.h>
#include <sys/types.h>


/* types */
typedef enum{
	F_CREATE = 0x1,
	F_APPEND = 0x2
} f_mode_t;

typedef struct{
	int fd;

	void *data;
	size_t data_len;

	int cmd;
	f_mode_t mode;

	int errno;
} sc_fs_t;


#endif // SYS_FILE_H
