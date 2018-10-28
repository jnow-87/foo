#include <sys/stat.h>
#include <sys/fcntl.h>
#include <lib/unistd.h>



/* global functions */
int stat(char const *path, stat_t *stat){
	int fd;


	fd = open(path, O_RDONLY);

	if(fd < 0 || fstat(fd, stat) != 0)
		return -errno;

	return close(fd);
}

int fstat(int fd, stat_t *stat){
	return fcntl(fd, F_STAT, stat, sizeof(stat_t));
}
