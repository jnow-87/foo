#include <arch/syscall.h>
#include <sys/syscall.h>
#include <lib/unistd.h>
#include <sys/string.h>
#include <sys/fcntl.h>


/* global functions */
int open(char const *path, f_mode_t mode){
	sc_fs_t p;


	if(path == 0x0 || *path == 0){
		errno = E_INVAL;
		return -1;
	}

	p.data = (void*)path;
	p.data_len = strlen(path) + 1;
	p.mode = mode;

	if(sc(SC_OPEN, &p) != E_OK)
		return -1;
	return p.fd;
}

int dup(int old_fd){
	return dup2(old_fd, -1);
}

int dup2(int old_fd, int new_fd){
	sc_fs_t p;


	p.fd = new_fd;
	p.data = (void*)old_fd;

	if(sc(SC_DUP, &p) != E_OK)
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

int unlink(char const *path){
	sc_fs_t p;


	p.data = (void*)path;
	p.data_len = strlen(path) + 1;

	if(sc(SC_RMNODE, &p) != E_OK)
		return -1;
	return 0;
}

int mkdir(char const *_path){
	size_t len = strlen(_path);
	char path[len + 2];
	int fd;


	strcpy(path, _path);

	if(_path[len - 1] != '/'){
		path[len] = '/';
		path[len + 1] = 0;
	}

	fd = open(path, O_CREAT);

	if(fd < 0)
		return -1;
	return close(fd);
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
	return unlink(path);
}
