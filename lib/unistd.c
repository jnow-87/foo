#include <arch/syscall.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/fcntl.h>
#include <sys/file.h>


/* global functions */
int open(char const *path, f_mode_t mode){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.mode = mode;

	errno = sc(SC_OPEN, &p);
	errno |= p.errno;

	return p.fd;
}

int close(int fd){
	sc_fs_t p;


	p.fd = fd;

	errno = sc(SC_CLOSE, &p);

	return_errno(p.errno);
}

int read(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;
	p.errno = E_OK;

	errno = sc(SC_READ, &p);
	errno |= p.errno;

	return p.data_len;
}

int write(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.data = buf;
	p.data_len = n;
	p.errno = E_OK;

	errno = sc(SC_WRITE, &p);
	errno |= p.errno;

	return p.data_len;
}

int ioctl(int fd, int cmd, void *data){
	sc_fs_t p;


	p.cmd = cmd;
	p.data = data;
	p.errno = E_OK;

	errno = sc(SC_IOCTL, &p);

	return_errno(p.errno);
}

int fcntl(int fd, int request, void *data){
	sc_fs_t p;


	p.cmd = request;
	p.data = data;
	p.errno = E_OK;

	errno = sc(SC_FCNTL, &p);

	return_errno(p.errno);
}

int chdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.errno = E_OK;

	errno = sc(SC_CHDIR, &p);

	return_errno(p.errno);
}

int rmdir(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.errno = E_OK;

	errno = sc(SC_RMNODE, &p);

	return_errno(p.errno);
}

int fseek(int fd, int offset, whence_t whence){
	seek_t p;


	p.whence = whence;
	p.offset = offset;

	if(fcntl(fd, F_SEEK, &p) != E_OK)
		return -1;
	return 0;
}

int ftell(int fd){
	seek_t p;


	fcntl(fd, F_TELL, &p);
	return p.pos;
}
