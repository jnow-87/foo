#include <config/config.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/devfs.h>
#include <kernel/kmem.h>
#include <sys/ringbuf.h>
#include <sys/uart.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>


/* local/static prototypes */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);


/* external variables */
extern uart_cbs_t uart_cbs;


/* global variables */
// configurations
uart_t uart_cfg[CONFIG_NUM_UART] = {{ 0 }};

// receiver buffer
ringbuf_t uart_rx_buf[CONFIG_NUM_UART] = {{ 0 }};
unsigned int uart_rx_err[CONFIG_NUM_UART] = { 0 };


/* static variables */
// devfs ids and ops
static int dev_ids[CONFIG_NUM_UART] = { 0 };
static devfs_ops_t dev_ops = {
	.open = 0x0,
	.close = 0x0,
	.read = read,
	.write = write,
	.ioctl = ioctl,
	.fcntl = 0x0
};


/* local functions */
static int kuart_init(void){
	unsigned int uart;


	for(uart=0; uart<CONFIG_NUM_UART; uart++){
		if(uart_cbs.config(uart, &kopt.uart_cfg) != E_OK)
			return errno;

		/* save config */
		uart_cfg[uart] = kopt.uart_cfg;
	}

	return E_OK;
}

platform_init(0, kuart_init);

static int _init(unsigned int uart){
	void *b;
	char name[] = "ttyx";


	/* allocate buffers */
	b = kmalloc(CONFIG_UART_RX_BUFSIZE);

	if(b == 0x0)
		goto err_0;

	ringbuf_init(uart_rx_buf + uart, b, CONFIG_UART_RX_BUFSIZE);

	/* register device */
	name[3] = '0' + uart;
	dev_ids[uart] = devfs_dev_register(name, &dev_ops, (void*)uart);

	if(dev_ids[uart] < 0)
		goto err_1;

	return E_OK;


err_1:
	kfree(b);

err_0:
	return errno;
}

static int init(void){
	unsigned int i;


	for(i=0; i<CONFIG_NUM_UART; i++)
		_init(i);

	return errno;
}

driver_init(init);

static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	unsigned int uart;


	uart = (unsigned int)(dev->data);

	if(uart_rx_err[uart]){
		errno = E_IO;
		uart_rx_err[uart] = 0;

		return 0;
	}

	return ringbuf_read(uart_rx_buf + uart, buf, n);
}

static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	unsigned int uart;


	uart = (unsigned int)(dev->data);

	if(uart_cbs.puts(uart, buf, n) != E_OK)
		return 0;
	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	unsigned int uart;


	uart = (unsigned int)(dev->data);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, uart_cfg + uart, sizeof(uart_t));

		// reset error flags
		uart_cfg[uart].frame_err = 0;
		uart_cfg[uart].data_overrun = 0;
		uart_cfg[uart].parity_err = 0;
		uart_cfg[uart].rx_queue_full = 0;

		uart_rx_err[uart] = 0;

		return E_OK;

	case IOCTL_CFGWR:
		if(uart_cbs.config(uart, (uart_t*)data) != E_OK)
			return errno;

		uart_cfg[uart] = *((uart_t*)data);
		return E_OK;

	default:
		return E_NOIMP;
	}
}
