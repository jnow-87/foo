#ifndef LIB_UNISTD_H
#define LIB_UNISTD_H


#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/file.h>


/* prototypes */
int open(char const *path, f_mode_t mode);
int close(int fd);

int read(int fd, void *buf, size_t n);
int write(int fd, void *buf, size_t n);

int ioctl(int fd, int cmd, void *data);
int fcntl(int fd, int request, void *data);

int chdir(char const *path);
int rmdir(char const *path);

int fseek(int fd, int offset, whence_t whence);
int ftell(int fd);


#endif // LIB_UNISTD_H
