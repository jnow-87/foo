/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/net.h>
#include <kernel/ksignal.h>
#include <kernel/csection.h>
#include <driver/term.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/net.h>
#include <sys/inet.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/patmat.h>


/* types */
// NOTE has to be aligned with rx_patterns
typedef enum{
	RESP_INVAL = -1,
	RESP_OK = 0,
	RESP_SENDOK,
	RESP_ERROR,
	RESP_READY,
	RESP_BUSY,
	RESP_DATA_IN,
	RESP_CLOSED,
	RESP_CONNECT,
//	RESP_CIPSTATUS,
} response_t;

typedef struct esp_t{
	socket_t *tcp_server,
			 *links[CONFIG_ESP8266_LINK_COUNT];

	term_itf_t *itf;

	ksignal_t sig;
	csection_lock_t lock;

	response_t resp;

	struct{
		void *(*hdlr)(struct esp_t *esp, char c);
		ringbuf_t esp_buf;
		patmat_t *parser;
		term_err_t err;

		socket_t *sock;
		size_t len,
			   idx;
		uint8_t *data;
	} rx;
} esp_t;


/* local/static prototypes */
static int configure(netdev_t *dev, void *cfg);

// socket callbacks
static int connect(socket_t *sock);
static int listen(socket_t *sock, int backlog);
static int close(socket_t *sock);
static ssize_t send(socket_t *sock, void *data, size_t data_len);

// esp input handling
static void rx_hdlr(int_num_t num, void *esp);
static void rx_task(void *esp);

static void *rx_parse(esp_t *esp, char c);
static void *rx_datain(esp_t *esp, char c);
static void *rx_skipline(esp_t *esp, char c);

static void prep_datain(esp_t *esp);
static void accept_connection(esp_t *esp);
static void close_socket(esp_t *esp);

static int cmd(esp_t *esp, response_t resp, char const *fmt, ...);
static void cmd_resp(esp_t *esp, ssize_t resp);
static int puts(term_itf_t *serial, char const *s);
static int get_link_id(esp_t *esp, socket_t *sock);


/* static variables */
static int const rx_patterns_specs = 4;
static char const *rx_patterns[] = {
	"OK",
	"SEND OK",
	"ERROR",
	"ready",
	"busy",
	"+IPD,%d,%d,%16s,%d:",
	"%d,CLOSED",
	"%d,CONNECT",
//	"+CIPSTATUS:%d,\"%4s\",\"%16s\",%d,",
};


/* global functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	void *esp_buf;
	esp_t *esp;
	devfs_dev_t *dev;
	netdev_ops_t ops;
	term_itf_t *itf;


	itf = (term_itf_t*)dt_itf;

	if(itf->rx_int == 0){
		FATAL("uart does not support rx interrupt\n");
		goto_errno(err_0, E_NOSUP);
	}

	/* register device */
	ops.configure = configure;
	ops.connect = connect;
	ops.listen = listen;
	ops.close = close;
	ops.send = send;

	esp = kmalloc(sizeof(esp_t));

	if(esp == 0x0)
		goto err_0;

	memset(esp, 0x0, sizeof(esp_t));

	esp->rx.parser = patmat_init(rx_patterns, sizeof_array(rx_patterns));

	if(esp->rx.parser == 0x0)
		goto err_1;

	esp_buf = kmalloc(16);

	if(esp_buf == 0x0)
		goto err_2;

	ringbuf_init(&esp->rx.esp_buf, esp_buf, 16);

	esp->itf = itf;

	ksignal_init(&esp->sig);
//	csection_init(&esp->lock);

	dev = netdev_register(name, &ops, AF_INET, esp);

	if(dev == 0x0)
		goto err_3;

	/* register interrupt */
	if(int_register(itf->rx_int, rx_hdlr, esp) != 0)
		goto err_4;

	/* configure hardware */
	if(itf->configure(dt_data, itf->data) != 0)
		goto err_5;

	esp->rx.err = 0;
	esp->rx.hdlr = rx_parse;

	if(ktask_create(rx_task, &esp, sizeof(esp_t*), 0x0, true) != 0)
		goto err_5;

	return E_OK;


err_5:
	int_release(itf->rx_int);

err_4:
	netdev_release(dev);

err_3:
	kfree(esp_buf);

err_2:
	patmat_destroy(esp->rx.parser);

err_1:
	kfree(esp);

err_0:
	return -errno;
}

device_probe("esp8266", probe);

// socket callbacks
static int configure(netdev_t *dev, void *_cfg){
	int r;
	char *enc[] ={"0", "2", "3", "4"};
	inetdev_cfg_t *cfg;
	esp_t *esp;


	cfg = (inetdev_cfg_t*)_cfg;
	esp = (esp_t*)dev->data;

	r = 0;

//	r |= cmd(esp, RESP_READY, "AT+RST");		// reset
	r |= cmd(esp, RESP_OK, "AT+CIPMUX=1");		// allow multiple connections

	r |= cmd(esp, RESP_OK, "AT+CWMODE_CUR=%s", (cfg->mode == INET_AP ? "2" : "1"));								// device mode
	r |= cmd(esp, RESP_OK, "AT+SYSMSG=2");																		// enable LINK_CONN message for incomming connections
	r |= cmd(esp, RESP_OK, "AT+CIPDINFO=1");																	// source ip and port for incoming messages
	r |= cmd(esp, RESP_OK, "AT+CWDHCP_CUR=%s,%s", (cfg->mode == INET_AP ? "0" : "1"), (cfg->dhcp ? "1" : "0"));	// dhcp

	if(cfg->mode == INET_AP){
		// configure access point
		r |= cmd(esp, RESP_OK, "AT+CIPAP_CUR=\"%a\",\"%a\",\"%a\"", &cfg->ip, &cfg->gw, &cfg->netmask);
		r |= cmd(esp, RESP_OK, "AT+CWSAP_CUR=\"%s\",\"%s\",1,%s,4,0", cfg->ssid, cfg->password, enc[cfg->enc]);
	}
	else if(cfg->mode == INET_CLIENT){
		// connect to access point
		if(!cfg->dhcp)
			r |= cmd(esp, RESP_OK, "AT+CIPSTA_CUR=\"%a\",\"%a\",\"%s\"", &cfg->ip, &cfg->netmask, &cfg->gw);

		r |= cmd(esp, RESP_OK, "AT+CWJAP_CUR=\"%s\",\"%s\"", cfg->ssid, cfg->password);
	}
	else
		r = -1;

	return r;
}

static int connect(socket_t *sock){
	esp_t *esp;
	inet_data_t *remote;
	int link_id;


	esp = (esp_t*)sock->dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	link_id = get_link_id(esp, 0x0);

	if(link_id < 0)
		return_errno(E_LIMIT);

	if(cmd(esp, RESP_OK, "AT+CIPSTART=%d,\"%s\",\"%a\",%d", link_id, (sock->type == SOCK_DGRAM ? "UDP" : "TCP"), &remote->addr, remote->port) != 0)
		return -errno;

	esp->links[link_id] = sock;

	return E_OK;
}

static int listen(socket_t *sock, int backlog){
	esp_t *esp;
	inet_data_t *remote;


	esp = (esp_t*)sock->dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	if(backlog != 0)
		return_errno(E_INVAL);

	if(esp->tcp_server != 0x0)
		return_errno(E_LIMIT);

	if(cmd(esp, RESP_OK, "AT+CIPSERVER=1,%d", remote->port) != 0)
		return -errno;

	esp->tcp_server = sock;

	return E_OK;
}

static int close(socket_t *sock){
	int link_id;
	esp_t *esp;


	esp = (esp_t*)sock->dev->data;

	if(sock == esp->tcp_server){
		if(cmd(esp, RESP_OK, "AT+CIPSERVER=0") != 0)
			return -errno;

		esp->tcp_server = 0x0;

		return E_OK;
	}

	link_id = get_link_id(esp, sock);

	if(link_id < 0)
		return_errno(E_INVAL);

	if(cmd(esp, RESP_OK, "AT+CIPCLOSE=%d", link_id) != 0)
		return -errno;

	esp->links[link_id] = 0x0;

	return E_OK;
}

static ssize_t send(socket_t *sock, void *data, size_t data_len){
	int r;
	int link_id;
	esp_t *esp;
	inet_data_t *remote;


	if(data_len == 0)
		return 0;

	esp = (esp_t*)sock->dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	r = 0;
	link_id = get_link_id(esp, sock);

	if(link_id < 0)
		return_errno(E_INVAL);

	if(sock->type == SOCK_DGRAM)	r |= cmd(esp, RESP_OK, "AT+CIPSEND=%d,%d,\"%a\",%d", link_id, data_len, &remote->addr, remote->port);
	else							r |= cmd(esp, RESP_OK, "AT+CIPSEND=%d,%d", link_id, data_len);

	if(r != 0)
		return -errno;

	r |= cmd(esp, RESP_SENDOK, "%x", data_len, data);

	return (r == 0 ? (ssize_t)data_len : -1);
}

// esp input handling
static void rx_hdlr(int_num_t num, void *_esp){
	size_t len;
	char buf[16];
	esp_t *esp;


	esp = (esp_t*)_esp;

	len = esp->itf->gets(buf, 16, &esp->rx.err, esp->itf->data);
	esp->rx.err = 0;	// TODO

	ringbuf_write(&esp->rx.esp_buf, buf, len);
}

static void rx_task(void *_esp){
	char c;
	esp_t *esp;


	esp = *((esp_t**)_esp);

	while(ringbuf_read(&esp->rx.esp_buf, &c, 1)){
//		DEBUG("%c\n", c);
		esp->rx.hdlr = esp->rx.hdlr(esp, c);
	}
}

static void *rx_parse(esp_t *esp, char c){
	ssize_t r;


	r = patmat_match_char(esp->rx.parser, c);

	if(r == PM_NOMATCH)
		return rx_skipline;

	if(r < 0)
		return rx_parse;

	DEBUG("match \"%s\"\n", rx_patterns[r]);

	switch(r){
	case RESP_OK:		// fall through
	case RESP_SENDOK:	// fall through
	case RESP_ERROR:	// fall through
		cmd_resp(esp, r);
		break;

	case RESP_DATA_IN:
		prep_datain(esp);
		return rx_datain;

	case RESP_CLOSED:
		close_socket(esp);
		break;

	case RESP_CONNECT:
		accept_connection(esp);
		break;

	case RESP_READY:	// ignored
	case RESP_BUSY:		// ignored
	default:
		break;
	}

	patmat_reset(esp->rx.parser);

	return rx_parse;
}

static void *rx_datain(esp_t *esp, char c){
	socket_t *sock;
	sock_addr_inet_t remote;
	void *res[rx_patterns_specs];


	sock = esp->rx.sock;

	if(sock->type == SOCK_DGRAM)	esp->rx.data[esp->rx.idx] = (uint8_t)c;
	else							socket_datain(sock, 0x0, 0, (uint8_t*)&c, 1);

	esp->rx.idx++;

	if(esp->rx.idx == esp->rx.len){
		if(sock->type == SOCK_DGRAM){
			(void)patmat_get_results(esp->rx.parser, res);

			remote.domain = AF_INET;
			remote.data.port = (uint16_t)(*((int*)res[3]));
			remote.data.addr = inet_addr(res[2]);

			socket_datain(sock, (sock_addr_t*)&remote, sizeof(sock_addr_inet_t), esp->rx.data, esp->rx.len);
			kfree(esp->rx.data);
		}

		patmat_reset(esp->rx.parser);

		return rx_parse;
	}

	return rx_datain;
}

static void *rx_skipline(esp_t *esp, char c){
	if(c == '\n'){
		patmat_reset(esp->rx.parser);
		return rx_parse;
	}

	return rx_skipline;
}

static void prep_datain(esp_t *esp){
	void *res[rx_patterns_specs];


	(void)patmat_get_results(esp->rx.parser, res);

	esp->rx.sock = esp->links[*((int*)res[0])];
	esp->rx.len = *((int*)res[1]);
	esp->rx.idx = 0;

	DEBUG("data: %d %d %s\n", esp->rx.len, *((int*)res[3]), res[2]);

	if(esp->rx.sock->type == SOCK_DGRAM)
		esp->rx.data = kmalloc(esp->rx.len);
}

static void accept_connection(esp_t *esp){
	void *res[rx_patterns_specs];
	sock_addr_inet_t remote;


	if(esp->tcp_server == 0x0)
		return;

	(void)patmat_get_results(esp->rx.parser, res);

	remote.domain = AF_INET;
// TODO
//	remote.data.port = (uint16_t)(*((int*)res[3]));
//	remote.data.addr = inet_addr(res[2]);

	(void)socket_add_client(esp->tcp_server, (sock_addr_t*)&remote, sizeof(sock_addr_inet_t));

//	TODO
//	DEBUG("connected: %s:%d\n", res[2], *((int*)res[1]));
}

static void close_socket(esp_t *esp){
	int link_id;
	void *res[rx_patterns_specs];


	(void)patmat_get_results(esp->rx.parser, res);

	link_id = *((int*)res[0]);
	DEBUG("disconnected: %d\n", link_id);

	socket_disconnect(esp->links[link_id]);
	esp->links[link_id] = 0x0;
}

// helper
static int get_link_id(esp_t *esp, socket_t *sock){
	uint8_t i;


	for(i=0; i<CONFIG_ESP8266_LINK_COUNT; i++){
		if(esp->links[i] == sock)
			return i;
	}

	return -1;
}

static int cmd(esp_t *esp, response_t resp, char const *fmt, ...){
	int r;
	char c;
	char s[16];
	size_t len;
	va_list lst;
	term_itf_t *itf;


	itf = esp->itf;
	r = 0;

	va_start(lst, fmt);
//	csection_lock(&esp->lock);

	DEBUG("cmd \"%s\"\n", fmt);
	esp->resp = RESP_INVAL;

	for(c=*fmt; c!=0; c=*(++fmt)){
		if(c == '%'){
			c = *(++fmt);

			if(c == 0)
				break;

			switch(c){
			case 'a':
				r |= puts(itf, inet_ntoa(*va_arg(lst, inet_addr_t*)));
				break;

			case 's':
				r |= puts(itf, va_arg(lst, char*));
				break;

			case 'x':
				len = va_arg(lst, size_t);
				r |= (itf->puts(va_arg(lst, char*), len, itf->data) != len);
				break;

			case 'd':
				snprintf(s, 16, "%d", va_arg(lst, int));
				r |= puts(itf, s);
				break;

			default:
				r |= (itf->putc(c, itf->data) != c);
			}
		}
		else
			r |= (itf->putc(c, itf->data) != c);
	}

	r |= (itf->putc('\r', itf->data) != '\r');
	r |= (itf->putc('\n', itf->data) != '\n');

	if(r != 0)
		goto_errno(end, E_IO);

	ksignal_wait(&esp->sig);
	DEBUG("res %d\n", esp->resp);

	if(esp->resp != resp){
		r = 1;

		if(esp->resp == RESP_BUSY)	errno = E_INUSE;
		else						errno = E_IO;
	}

end:
	va_end(lst);
//	csection_unlock(&esp->lock);

	return (r == 0 ? 0 : -1);
}

static void cmd_resp(esp_t *esp, ssize_t resp){
	esp->resp = resp;
	ksignal_send(&esp->sig);
}

static int puts(term_itf_t *serial, char const *s){
	size_t n;


	n = strlen(s);

	if(serial->puts(s, n, serial->data) != n)
		return -1;
	return 0;
}
