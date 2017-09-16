#include <kernel/init.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>
#include <sys/string.h>


/* macros */
#define LOOP_BUF_SIZE	16


/* local variables */
static int loop_dev_id = 0;
static char loop_buf[LOOP_BUF_SIZE];


/* local/static prototypes */
static int loop_open(int id, fs_filed_t *fd, f_mode_t mode);
static int loop_close(int id);
static int loop_read(int id, fs_filed_t *fd, void *buf, size_t n);
static int loop_write(int id, fs_filed_t *fd, void *buf, size_t n);
static int loop_ioctl(int id, fs_filed_t *fd, int request, void *data);
static int loop_fcntl(int id, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int loop_init(void){
	devfs_ops_t ops;


	ops.open = loop_open;
	ops.close = loop_close;
	ops.read = loop_read;
	ops.write = loop_write;
	ops.ioctl = loop_ioctl;
	ops.fcntl = loop_fcntl;

	loop_dev_id = devfs_dev_register("loop", &ops);

	if(loop_dev_id < 0)
		return errno;
	return E_OK;
}

driver_init(loop_init);

static int loop_open(int id, fs_filed_t *fd, f_mode_t mode){
	DEBUG("dummy callback for loop device\n");
	return fd->id;
}

static int loop_close(int id){
	DEBUG("dummy callback for loop device\n");
	return E_OK;
}

static int loop_read(int id, fs_filed_t *fd, void *buf, size_t n){
	if(n > LOOP_BUF_SIZE)
		n = LOOP_BUF_SIZE;

	DEBUG("copy from loop buffer \"%*.*s\"\n", n, n, loop_buf);
	memcpy(buf, loop_buf, n);

	return n;
}

static int loop_write(int id, fs_filed_t *fd, void *buf, size_t n){
	if(n > LOOP_BUF_SIZE){
		DEBUG("data too large for loop buffer\n");
		return_errno(E_LIMIT);
	}

	DEBUG("copy to buffer \"%*.*s\"\n", n, n, buf);
	memcpy(loop_buf, buf, n);

	return n;
}

static int loop_ioctl(int id, fs_filed_t *fd, int request, void *data){
	DEBUG("dummy callback for loop device\n");
	return E_OK;
}

static int loop_fcntl(int id, fs_filed_t *fd, int cmd, void *data){
	DEBUG("dummy callback for loop device\n");
	return E_OK;
}
