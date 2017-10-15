#ifndef SYS_FILE_H
#define SYS_FILE_H


#include <sys/errno.h>
#include <sys/types.h>


/* types */
typedef enum{
	O_CREATE = 0x1,
	O_APPEND = 0x2,
	O_READ = 0x4,
	O_WRITE = 0x8,
} f_mode_t;


#endif // SYS_FILE_H
