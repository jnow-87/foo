/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H


#include <sys/binloader.h>
#include <sys/process.h>
#include <sys/thread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/net.h>


/* types */
// syscall common
typedef enum{
	SC_OPEN,
	SC_DUP,
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
	SC_PROCCREATE,
	SC_PROCINFO,
	SC_THREADCREATE,
	SC_THREADINFO,
	SC_NICE,
	SC_SCHEDYIELD,
	SC_SIGREGISTER,
	SC_SIGSEND,
	SC_SIGRETURN,
	SC_SLEEP,
	SC_TIME,
	SC_SOCKET,
	SC_RECV,
	SC_SEND,
	NSYSCALLS
} sc_num_t;

typedef struct{
	sc_num_t num;

	void *param;
	size_t size;
} sc_t;

// syscall specific arguments
typedef struct{
	void *p;
	size_t size;
} sc_malloc_t;

typedef struct{
	int fd;

	void *data;
	size_t data_len;

	int cmd;
	f_mode_t mode;
} sc_fs_t;

typedef struct{
	int status;
	bool kill_siblings;
} sc_exit_t;

typedef struct{
	pid_t pid;

	void *binary;
	bin_type_t bin_type;

	char const *name,
			   *args;
	size_t name_len,
		   args_len;
} sc_process_t;

typedef struct{
	tid_t tid;

	thread_entry_t entry;
	void *arg;

	unsigned int affinity;
	int priority;
} sc_thread_t;

typedef struct{
	pid_t pid;
	tid_t tid;

	signal_t sig;
	thread_entry_t hdlr;
} sc_signal_t;

typedef struct{
	time_t time;
} sc_time_t;

typedef struct{
	int fd;

	sock_type_t type;
	sock_addr_t *addr;
	size_t addr_len;

	void *data;
	size_t data_len;
} sc_socket_t;


#endif // SYS_SYSCALL_H
