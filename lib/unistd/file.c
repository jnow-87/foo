/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <sys/syscall.h>
#include <lib/unistd.h>
#include <lib/stdio.h>
#include <sys/string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>


/* global functions */
int open(char const *path, f_mode_t mode){
	sc_fs_t p;


	if(path == 0x0 || *path == 0){
		set_errno(E_INVAL);

		return -1;
	}

	p.payload = (void*)path;
	p.payload_len = strlen(path) + 1;
	p.mode = mode;

	if(sc(SC_OPEN, &p) != 0)
		return -1;

	return p.fd;
}

int dup(int old_fd){
	return dup2(old_fd, -1);
}

int dup2(int old_fd, int new_fd){
	sc_fs_t p;


	p.fd = new_fd;
	p.payload = &old_fd;

	if(sc(SC_DUP, &p) != 0)
		return -1;

	return p.fd;
}

int close(int fd){
	sc_fs_t p;


	p.fd = fd;

	if(sc(SC_CLOSE, &p) != 0)
		return -1;

	return 0;
}

ssize_t read(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.payload = buf;
	p.payload_len = n;

	if(sc(SC_READ, &p) != 0)
		return -1;

	return p.payload_len;
}

ssize_t write(int fd, void *buf, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.payload = buf;
	p.payload_len = n;

	if(sc(SC_WRITE, &p) != 0)
		return -1;

	return p.payload_len;
}

int ionctl(int fd, int cmd, void *arg, size_t arg_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = cmd;
	p.payload = arg;
	p.payload_len = arg_len;

	if(sc(SC_IOCTL, &p) != 0)
		return -1;

	return 0;
}

int fcntl(int fd, int request, void *arg, size_t arg_len){
	sc_fs_t p;


	p.fd = fd;
	p.cmd = request;
	p.payload = arg;
	p.payload_len = arg_len;

	if(sc(SC_FCNTL, &p) != 0)
		return -1;

	return 0;
}

void *mmap(int fd, size_t n){
	sc_fs_t p;


	p.fd = fd;
	p.payload = 0x0;
	p.payload_len = n;

	if(sc(SC_MMAP, &p) != 0)
		return 0x0;

	return p.payload;
}

void munmap(void *addr){
	sc_fs_t p;


	p.payload = addr;

	(void)sc(SC_MMAP, &p);
}

int unlink(char const *path){
	sc_fs_t p;


	p.payload = (void*)path;
	p.payload_len = strlen(path) + 1;

	if(sc(SC_RMNODE, &p) != 0)
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


	p.payload = (void*)path;
	p.payload_len = strlen(path) + 1;

	if(sc(SC_CHDIR, &p) != 0)
		return -1;

	return 0;
}

int rmdir(char const *path){
	stat_t f_stat;


	if(stat(path, &f_stat) != 0 || f_stat.type != FT_DIR)
		return_errno(E_INVAL);

	return unlink(path);
}

int stat(char const *path, stat_t *stat){
	int fd;


	fd = open(path, O_RDONLY);

	if(fd < 0 || fstat(fd, stat) != 0)
		return -errno;

	return close(fd);
}

int statat(char const *dir, char const *path, stat_t *_stat){
	char _path[strlen(dir) + strlen(path) + 2];


	if(dir == 0x0 || path == 0x0)
		return_errno(E_INVAL);

	sprintf(_path, "%s/%s", dir, path);

	return stat(_path, _stat);
}

int fstat(int fd, stat_t *stat){
	return fcntl(fd, F_STAT, stat, sizeof(stat_t));
}
