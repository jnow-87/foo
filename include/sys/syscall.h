#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H


#include <arch/syscall.h>
#include <sys/compiler.h>


/* macros */
// needs to be a macro, otherwise sizeof(*param)
// wouldn't work
#define syscall(num, param) \
	arch_syscall(num, param, sizeof(*param))


/* types */
typedef enum __packed{
	NSYSCALLS
} syscall_t;


#endif // SYS_SYSCALL_H
