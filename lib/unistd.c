#include <arch/syscall.h>
#include <arch/thread.h>
#include <lib/unistd.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/binloader.h>
#include <sys/fcntl.h>
#include <sys/file.h>


/* global functions */
int open(char const *path, f_mode_t mode){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.mode = mode;

	sc(SC_OPEN, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return p.fd;
}

int close(int fd){
	sc_fs_t p;


	p.fd = fd;

	sc(SC_CLOSE, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return 0;
}

ssize_t read(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;
	p.errno = E_OK;

	sc(SC_READ, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return p.data_len;
}

ssize_t write(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;
	p.errno = E_OK;

	sc(SC_WRITE, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return p.data_len;
}

int ioctl(int fd, int cmd, void *data, size_t data_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = cmd;
	p.data = data;
	p.data_len = data_len;
	p.errno = E_OK;

	sc(SC_IOCTL, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return 0;
}

int fcntl(int fd, int request, void *data, size_t data_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = request;
	p.data = data;
	p.data_len = data_len;
	p.errno = E_OK;

	sc(SC_FCNTL, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return 0;
}

int chdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.errno = E_OK;

	sc(SC_CHDIR, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return 0;
}

int rmdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.errno = E_OK;

	sc(SC_RMNODE, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return -1;
	return 0;
}

thread_id_t thread_create(int (*entry)(void *), void *arg){
	sc_thread_t p;


	p.entry = entry;
	p.arg = arg;

	sc(SC_THREADCREATE, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return 0;
	return p.tid;
}

int thread_info(thread_info_t *info){
	sc_thread_t p;


	sc(SC_THREADINFO, &p);
	errno |= p.errno;

	info->tid = p.tid;
	info->priority = p.priority;
	info->affinity = p.affinity;

	return p.errno;
}

int nice(int inc){
	sc_thread_t p;


	p.priority = inc;
	sc(SC_NICE, &p);

	return (p.errno != E_OK) ? p.errno : p.priority;
}

process_id_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args){
	sc_process_t p;


	p.binary = binary;
	p.bin_type = bin_type;
	p.name = name;
	p.name_len = strlen(name);
	p.args = args;
	p.args_len = strlen(args);

	sc(SC_PROCCREATE, &p);
	errno |= p.errno;

	if(p.errno != E_OK)
		return 0;
	return p.pid;
}

int process_info(process_info_t *info){
	sc_process_t p;

	sc(SC_PROCINFO, &p);
	errno |= p.errno;

	info->pid = p.pid;

	return p.errno;
}
