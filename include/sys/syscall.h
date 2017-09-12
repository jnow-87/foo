#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H


/* types */
typedef enum{
	SC_OPEN,
	SC_CLOSE,
	SC_READ,
	SC_WRITE,
	SC_IOCTL,
	SC_FCNTL,
	SC_RMNODE,
	SC_CHDIR,
	NSYSCALLS
} sc_t;


#endif // SYS_SYSCALL_H
