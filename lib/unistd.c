#include <arch/syscall.h>
#include <lib/unistd.h>
#include <sys/string.h>
#include <sys/syscall.h>
#include <sys/binloader.h>
#include <sys/thread.h>
#include <sys/fcntl.h>
#include <sys/file.h>


/* global functions */
int open(char const *path, f_mode_t mode){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.mode = mode;

	if(sc(SC_OPEN, &p) != E_OK)
		return -1;
	return p.fd;
}

int close(int fd){
	sc_fs_t p;


	p.fd = fd;

	if(sc(SC_CLOSE, &p) != E_OK)
		return -1;
	return 0;
}

ssize_t read(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;

	if(sc(SC_READ, &p) != E_OK)
		return -1;
	return p.data_len;
}

ssize_t write(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;

	if(sc(SC_WRITE, &p) != E_OK)
		return -1;
	return p.data_len;
}

int ioctl(int fd, int cmd, void *data, size_t data_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = cmd;
	p.data = data;
	p.data_len = data_len;

	if(sc(SC_IOCTL, &p) != E_OK)
		return -1;
	return 0;
}

int fcntl(int fd, int request, void *data, size_t data_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = request;
	p.data = data;
	p.data_len = data_len;

	if(sc(SC_FCNTL, &p) != E_OK)
		return -1;
	return 0;
}

int chdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;

	if(sc(SC_CHDIR, &p) != E_OK)
		return -1;
	return 0;
}

int rmdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;

	if(sc(SC_RMNODE, &p) != E_OK)
		return -1;
	return 0;
}

tid_t thread_create(int (*entry)(void *), void *arg){
	sc_thread_t p;


	p.entry = entry;
	p.arg = arg;

	if(sc(SC_THREADCREATE, &p) != E_OK)
		return 0;
	return p.tid;
}

int thread_info(thread_info_t *info){
	int r;
	sc_thread_t p;


	r = sc(SC_THREADINFO, &p);

	info->tid = p.tid;
	info->priority = p.priority;
	info->affinity = p.affinity;

	return r;
}

int nice(int inc){
	int r;
	sc_thread_t p;


	p.priority = inc;
	r = sc(SC_NICE, &p);

	if(r != E_OK)
		return r;
	return p.priority;
}

pid_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args){
	sc_process_t p;


	p.binary = binary;
	p.bin_type = bin_type;
	p.name = name;
	p.name_len = strlen(name);
	p.args = args;
	p.args_len = strlen(args);

	if(sc(SC_PROCCREATE, &p) != E_OK)
		return 0;
	return p.pid;
}

int process_info(process_info_t *info){
	int r;
	sc_process_t p;


	r = sc(SC_PROCINFO, &p);

	info->pid = p.pid;

	return r;
}

int sleep(size_t ms, size_t us){
	sc_time_t p;


	p.time.us = us;
	p.time.ms = ms;

	return sc(SC_SLEEP, &p);
}
