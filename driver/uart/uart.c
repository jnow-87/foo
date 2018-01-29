#include <config/config.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/devfs.h>
#include <kernel/kmem.h>
#include <kernel/signal.h>
#include <driver/uart.h>
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
// uarts
kuart_t uarts[CONFIG_UART_CNT] = { 0 };


/* static variables */
// devfs ids and ops
static devfs_dev_t *devs[CONFIG_UART_CNT] = { 0 };


/* local functions */
static int kuart_init(void){
	unsigned int i;


	for(i=0; i<CONFIG_UART_CNT; i++){
		if(uart_cbs.config(i, &kopt.uart_cfg) != E_OK)
			return -errno;

		/* save config */
		uarts[i].cfg = kopt.uart_cfg;
	}

	return E_OK;
}

platform_init(0, kuart_init);

static int _init(unsigned int uart){
	void *b;
	char name[] = "ttyx";
	devfs_ops_t ops;


	/* init uart */
	b = kmalloc(CONFIG_UART_RX_BUFSIZE);

	if(b == 0x0)
		goto err_0;

	ringbuf_init(&uarts[uart].rx_buf, b, CONFIG_UART_RX_BUFSIZE);

	uarts[uart].rx_err = 0;
	ksignal_init(&uarts[uart].rx_rdy);

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	name[3] = '0' + uart;
	devs[uart] = devfs_dev_register(name, &ops, (void*)uart);

	if(devs[uart] == 0x0)
		goto err_1;

	return E_OK;


err_1:
	kfree(b);

err_0:
	return -errno;
}

static int init(void){
	unsigned int i;


	for(i=0; i<CONFIG_UART_CNT; i++)
		_init(i);

	return -errno;
}

driver_init(init);

static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	unsigned int uart;


	uart = (unsigned int)(dev->data);

	if(uarts[uart].rx_err){
		errno = E_IO;
		uarts[uart].rx_err = 0;

		return 0;
	}

	if(uarts[uart].cfg.blocking && ringbuf_empty(&uarts[uart].rx_buf))
		ksignal_wait(&uarts[uart].rx_rdy);

	return ringbuf_read(&uarts[uart].rx_buf, buf, n);
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
		memcpy(data, &uarts[uart].cfg, sizeof(uart_t));

		// reset error flags
		uarts[uart].cfg.frame_err = 0;
		uarts[uart].cfg.data_overrun = 0;
		uarts[uart].cfg.parity_err = 0;
		uarts[uart].cfg.rx_queue_full = 0;

		uarts[uart].rx_err = 0;

		return E_OK;

	case IOCTL_CFGWR:
		if(uart_cbs.config(uart, (uart_t*)data) != E_OK)
			return -errno;

		uarts[uart].cfg = *((uart_t*)data);
		return E_OK;

	default:
		return_errno(E_NOIMP);
	}
}
