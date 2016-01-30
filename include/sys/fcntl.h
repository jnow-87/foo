#ifndef SYS_FCNTL_H
#define SYS_FCNTL_H


#include <sys/compiler.h>


/* types */
typedef enum __packed{
	F_CREATE = 1,
	F_APPEND
} f_mode_t;


#endif // SYS_FCNTL_H
