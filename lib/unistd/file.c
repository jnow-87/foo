#include <arch/syscall.h>
#include <sys/syscall.h>
#include <lib/unistd.h>
#include <sys/string.h>
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
