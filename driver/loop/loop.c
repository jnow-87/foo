#include <kernel/init.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <kernel/kmem.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/ringbuf.h>


/* local variables */
static int loop_dev_id = 0;
static ringbuf_t loop_buf;


/* local/static prototypes */
static int loop_open(int id, fs_filed_t *fd, f_mode_t mode);
static int loop_close(int id);
static int loop_read(int id, fs_filed_t *fd, void *buf, size_t n);
static int loop_write(int id, fs_filed_t *fd, void *buf, size_t n);
static int loop_ioctl(int id, fs_filed_t *fd, int request, void *data);
static int loop_fcntl(int id, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int loop_init(void){
	char *b;
	devfs_ops_t ops;


	/* init device buffer */
	b = kmalloc(CONFIG_LOOP_BUF_SIZE);

	if(b == 0x0)
		return_errno(E_NOMEM);

	ringbuf_init(&loop_buf, b, CONFIG_LOOP_BUF_SIZE);

	/* register device */
	ops.open = loop_open;
	ops.close = loop_close;
	ops.read = loop_read;
	ops.write = loop_write;
	ops.ioctl = loop_ioctl;
	ops.fcntl = loop_fcntl;

	loop_dev_id = devfs_dev_register("loop", &ops);

	if(loop_dev_id < 0)
		goto err;

	return E_OK;


err:
	kfree(b);
	return errno;
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
	n = ringbuf_read(&loop_buf, buf, n);
	DEBUG("copy from loop buffer \"%*.*s\"\n", n, n, buf);

	return n;
}

static int loop_write(int id, fs_filed_t *fd, void *buf, size_t n){
	n = ringbuf_write(&loop_buf, buf, n);

	DEBUG("copy to buffer \"%*.*s\"\n", n, n, buf);

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
