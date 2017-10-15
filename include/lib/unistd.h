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

int ioctl(int fd, int cmd, void *data, size_t data_len);
int fcntl(int fd, int request, void *data, size_t data_len);

int chdir(char const *path);
int rmdir(char const *path);


#endif // LIB_UNISTD_H
