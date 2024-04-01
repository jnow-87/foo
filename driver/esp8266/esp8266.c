/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/interrupt.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/ksignal.h>
#include <kernel/inttask.h>
#include <kernel/net.h>
#include <driver/term.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
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
	RESP_FAIL,
	RESP_READY,
	RESP_BUSY,
	RESP_DATA_IN,
	RESP_CLOSED,
	RESP_CONNECT,
	RESP_IP_INFO,
} response_t;

typedef struct{
	char const *s;
	size_t len;

	term_itf_t *term;
} tx_dgram_t;

typedef struct esp_t{
	term_itf_t *term;

	inetdev_cfg_t cfg;

	socket_t *tcp_server,
			 *links[CONFIG_ESP8266_LINK_COUNT];

	ksignal_t sig;
	response_t resp;

	mutex_t mtx;

	struct{
		void *(*hdlr)(struct esp_t *esp, char c);
		ringbuf_t line;
		patmat_t *parser;

		socket_t *sock;
		size_t len,
			   idx;
		uint8_t *dgram;
	} rx;

	itask_queue_t tx_queue;
} esp_t;


/* local/static prototypes */
// netfs callbacks
static int configure(netdev_t *dev);
static int connect(socket_t *sock);
static int bind(socket_t *sock);
static int listen(socket_t *sock);
static void close(socket_t *sock);
static ssize_t send(socket_t *sock, void *buf, size_t n);
static int _connect(socket_t *sock, inet_addr_t *addr, uint16_t remote_port, uint16_t local_port);

// rx interrupt handling
static void rx_hdlr(int_num_t num, void *payload);
static void rx_task(void *payload);

static void *rx_parse(esp_t *esp, char c);
static void *rx_skipline(esp_t *esp, char c);
static void *rx_datain_stream(esp_t *esp, char c);
static void *rx_datain_dgram(esp_t *esp, char c);

static void *rx_act_pre_datain(esp_t *esp);
static void rx_act_accept(esp_t *esp);
static void rx_act_close(esp_t *esp);
static void rx_act_ip_info(esp_t *esp);

// tx interrupt handling
static void tx_hdlr(int_num_t num, void *payload);
static int tx_complete(void *payload);

// command handling
static int cmd(esp_t *esp, response_t resp, bool skip, char const *fmt, ...);
static void cmd_resp(esp_t *esp, ssize_t resp);
static int puts(char const *s, esp_t *esp);
static int putsn(char const *s, size_t n, esp_t *esp);

// utilities
static int get_link_id(esp_t *esp, socket_t *sock);


/* static variables */
static char const *rx_patterns[] = {
	"OK",
	"SEND OK",
	"ERROR",
	"FAIL",
	"ready",
	"busy",
	"+IPD,%d,%d,%16s,%d:",
	"%d,CLOSED",
	"+LINK_CONN:%d,%d,\"%4s\",%d,\"%16s\",%d,",
	"+CIPSTA_CUR:%8s:\"%16s\"",
};


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_itf_t *term = (term_itf_t*)dt_itf;
	void *rxbuf;
	esp_t *esp;
	devfs_dev_t *dev;
	netdev_itf_t itf;
	term_cfg_t term_cfg;


	if(term->rx_int == 0 || term->tx_int == 0){
		WARN("uart does not support interrupts\n");
		goto_errno(err_0, E_NOSUP);
	}

	/* create net device */
	esp = kcalloc(1, sizeof(esp_t));

	if(esp == 0x0)
		goto err_0;

	esp->rx.parser = patmat_init(rx_patterns, sizeof_array(rx_patterns));

	if(esp->rx.parser == 0x0)
		goto err_1;

	rxbuf = kmalloc(16);

	if(rxbuf == 0x0)
		goto err_2;

	ringbuf_init(&esp->rx.line, rxbuf, 16);

	esp->term = term;
	esp->rx.hdlr = rx_parse;
	esp->resp = RESP_INVAL;

	ksignal_init(&esp->sig);
	mutex_init(&esp->mtx, MTX_NOINT);
	itask_queue_init(&esp->tx_queue);

	itf.configure = configure;
	itf.connect = connect;
	itf.bind = bind;
	itf.listen = listen;
	itf.close = close;
	itf.send = send;
	itf.cfg = &esp->cfg;

	dev = netdev_register(name, AF_INET, &itf, esp);

	if(dev == 0x0)
		goto err_3;

	/* configure hardware */
	if(int_register(term->rx_int, rx_hdlr, esp) != 0)
		goto err_4;

	if(int_register(term->tx_int, tx_hdlr, esp) != 0)
		goto err_4;

	if(term->rx_int)
		dev->node->timeout_us = 0;

	if(term->configure != 0x0 && term->configure(&term_cfg, term->cfg, term->hw) != 0)
		goto err_5;

	if(ktask_create(rx_task, &esp, sizeof(esp_t*), 0x0, true) != 0)
		goto err_5;

	return 0x0;


err_5:
	int_release(term->tx_int);
	int_release(term->rx_int);

err_4:
	netdev_release(dev);

err_3:
	kfree(rxbuf);

err_2:
	patmat_destroy(esp->rx.parser);

err_1:
	kfree(esp);

err_0:
	return 0x0;
}

driver_probe("esp8266", probe);

// socket callbacks
static int configure(netdev_t *dev){
	esp_t *esp = (esp_t*)dev->payload;
	inetdev_cfg_t *cfg = &esp->cfg;
	char *enc[] = { "0", "2", "3", "4" };
	int r = 0;


	r |= cmd(esp, RESP_OK, r, "ATE0");					// disable echo
	r |= cmd(esp, RESP_OK, r, "AT+CIPMUX=1");			// allow multiple connections
	r |= cmd(esp, RESP_OK, r, "AT+CWMODE_CUR=%s",		// device mode
		(cfg->mode == INET_AP ? "2" : "1")
	);

	r |= cmd(esp, RESP_OK, r, "AT+SYSMSG=2");			// enable LINK_CONN message for incomming connections
	r |= cmd(esp, RESP_OK, r, "AT+CIPDINFO=1");			// source ip and port for incoming messages
	r |= cmd(esp, RESP_OK, r, "AT+CWDHCP_CUR=%s,%s",	// dhcp
		(cfg->mode == INET_AP ? "0" : "1"),
		(cfg->dhcp ? "1" : "0")
	);

	if(cfg->mode == INET_AP){
		// configure access point
		r |= cmd(esp, RESP_OK, r, "AT+CIPAP_CUR=\"%a\",\"%a\",\"%a\"", &cfg->ip, &cfg->gateway, &cfg->netmask);
		r |= cmd(esp, RESP_OK, r, "AT+CWSAP_CUR=\"%s\",\"%s\",1,%s,4,0", cfg->ssid, cfg->password, enc[cfg->enc]);
	}
	else if(cfg->mode == INET_CLIENT){
		// connect to access point
		if(!cfg->dhcp)
			r |= cmd(esp, RESP_OK, r, "AT+CIPSTA_CUR=\"%a\",\"%a\",\"%s\"", &cfg->ip, &cfg->netmask, &cfg->gateway);

		r |= cmd(esp, RESP_OK, r, "AT+CWJAP_CUR=\"%s\",\"%s\"", cfg->ssid, cfg->password);
	}
	else
		r = -1;

	r |= cmd(esp, RESP_OK, r, "AT+CIPSTA_CUR?");

	return r;
}

static int connect(socket_t *sock){
	inet_data_t *remote = &((sock_addr_inet_t*)(&sock->addr))->inet_data;


	return _connect(sock, &remote->addr, remote->port, 0);
}

static int bind(socket_t *sock){
	inet_data_t *remote = &((sock_addr_inet_t*)(&sock->addr))->inet_data;


	if(sock->type == SOCK_STREAM)
		return 0;

	return _connect(sock, &remote->addr, 0, remote->port);
}

static int listen(socket_t *sock){
	esp_t *esp = (esp_t*)sock->dev->payload;
	inet_data_t *remote = &((sock_addr_inet_t*)(&sock->addr))->inet_data;


	mutex_lock(&esp->mtx);

	if(esp->tcp_server != 0x0)
		goto_errno(end, E_LIMIT);

	if(cmd(esp, RESP_OK, false, "AT+CIPSERVER=1,%u", remote->port) != 0)
		goto end;

	esp->tcp_server = sock;

end:
	mutex_unlock(&esp->mtx);

	return 0;
}

static void close(socket_t *sock){
	esp_t *esp = (esp_t*)sock->dev->payload;
	int link_id;


	mutex_lock(&esp->mtx);

	if(sock == esp->tcp_server){
		if(cmd(esp, RESP_OK, false, "AT+CIPSERVER=0") == 0)
			esp->tcp_server = 0x0;
	}
	else{
		link_id = get_link_id(esp, sock);

		if(link_id >= 0 && cmd(esp, RESP_OK, false, "AT+CIPCLOSE=%u", link_id) == 0)
			esp->links[link_id] = 0x0;
	}

	mutex_unlock(&esp->mtx);
}

static ssize_t send(socket_t *sock, void *buf, size_t n){
	esp_t *esp = (esp_t*)sock->dev->payload;
	inet_data_t *remote = &((sock_addr_inet_t*)(&sock->addr))->inet_data;
	int r = 0;
	int link_id;


	if(n == 0)
		return 0;

	mutex_lock(&esp->mtx);
	link_id = get_link_id(esp, sock);
	mutex_unlock(&esp->mtx);

	if(link_id < 0)
		return_errno(E_NOCONN);

	if(sock->type == SOCK_DGRAM)	r |= cmd(esp, RESP_OK, r, "AT+CIPSEND=%u,%u,\"%a\",%u", link_id, n, &remote->addr, remote->port);
	else							r |= cmd(esp, RESP_OK, r, "AT+CIPSEND=%u,%u", link_id, n);

	if(r == 0)
		r |= cmd(esp, RESP_SENDOK, r, "%x", n, buf);

	return (r == 0 ? (ssize_t)n : -1);
}

static int _connect(socket_t *sock, inet_addr_t *addr, uint16_t remote_port, uint16_t local_port){
	esp_t *esp = (esp_t*)sock->dev->payload;
	int link_id;
	int r;


	mutex_lock(&esp->mtx);
	link_id = get_link_id(esp, 0x0);
	mutex_unlock(&esp->mtx);

	if(link_id < 0)
		return_errno(E_LIMIT);

	if(remote_port == 0)
		remote_port = link_id + 1;

	if(local_port == 0)
		local_port = link_id + 1;

	r = cmd(esp, RESP_OK, false, "AT+CIPSTART=%u,\"%s\",\"%a\",%u,%u",
		link_id,
		(sock->type == SOCK_DGRAM ? "UDP" : "TCP"),
		addr,
		remote_port,
		local_port
	);

	if(r != 0)
		return -errno;

	mutex_lock(&esp->mtx);
	esp->links[link_id] = sock;
	mutex_unlock(&esp->mtx);

	return 0;
}

// rx interrupt handling
static void rx_hdlr(int_num_t num, void *payload){
	esp_t *esp = (esp_t*)payload;
	size_t len;
	char buf[16];


	mutex_lock(&esp->mtx);

	len = esp->term->gets(buf, 16, esp->term->hw);

	if(len > 0)
		ringbuf_write(&esp->rx.line, buf, len);

	mutex_unlock(&esp->mtx);
}

static void rx_task(void *payload){
	esp_t *esp = *((esp_t**)payload);
	char c;
	size_t n;


	while(1){
		mutex_lock(&esp->mtx);
		n = ringbuf_read(&esp->rx.line, &c, 1);
		mutex_unlock(&esp->mtx);

		if(n == 0)
			break;

		esp->rx.hdlr = esp->rx.hdlr(esp, c);
	}
}

static void *rx_parse(esp_t *esp, char c){
	ssize_t r;


	r = patmat_match_char(esp->rx.parser, c);

	if(r == PM_NOMATCH)
		return rx_skipline(esp, c);

	if(r < 0)
		return rx_parse;

	DEBUG("match \"%s\"\n", rx_patterns[r]);

	switch(r){
	case RESP_OK:		// fall through
	case RESP_SENDOK:	// fall through
	case RESP_ERROR:	// fall through
	case RESP_FAIL:		// fall through
		cmd_resp(esp, r);
		break;

	case RESP_DATA_IN:
		return rx_act_pre_datain(esp);

	case RESP_CLOSED:
		rx_act_close(esp);
		break;

	case RESP_CONNECT:
		rx_act_accept(esp);
		break;

	case RESP_IP_INFO:
		rx_act_ip_info(esp);
		break;

	case RESP_READY:	// ignored
	case RESP_BUSY:		// ignored
	default:
		break;
	}

	patmat_reset(esp->rx.parser);

	return rx_parse;
}

static void *rx_skipline(esp_t *esp, char c){
	if(c == '\n'){
		patmat_reset(esp->rx.parser);
		return rx_parse;
	}

	return rx_skipline;
}

static void *rx_datain_stream(esp_t *esp, char c){
	bool is_last = (esp->rx.idx == esp->rx.len);
	socket_t *sock = esp->rx.sock;


	esp->rx.idx++;

	if(is_last)
		patmat_reset(esp->rx.parser);

	if(socket_datain_stream(sock, (uint8_t*)&c, 1, is_last) != 0)
		WARN("data loss: %s\n", strerror(errno));

	if(!is_last)
		return rx_datain_stream;

	return rx_parse;
}

static void *rx_datain_dgram(esp_t *esp, char c){
	void *res[4];
	void *rx_hdlr = rx_datain_dgram;
	socket_t *sock = esp->rx.sock;
	sock_addr_inet_t remote;


	esp->rx.dgram[esp->rx.idx] = (uint8_t)c;
	esp->rx.idx++;

	if(esp->rx.idx == esp->rx.len){
		(void)patmat_get_results(esp->rx.parser, res);

		remote.domain = AF_INET;
		remote.inet_data.port = (uint16_t)PATMAT_RESULT_INT(res, 3);
		remote.inet_data.addr = inet_addr(PATMAT_RESULT_STR(res, 2));

		if(socket_datain_dgram(sock, esp->rx.dgram, esp->rx.len, (sock_addr_t*)&remote, sizeof(sock_addr_inet_t)) != 0)
			WARN("data loss: %s\n", strerror(errno));

		patmat_reset(esp->rx.parser);

		rx_hdlr = rx_parse;
	}

	return rx_hdlr;
}

static void *rx_act_pre_datain(esp_t *esp){
	void *res[4];
	socket_t *sock;


	(void)patmat_get_results(esp->rx.parser, res);

	mutex_lock(&esp->mtx);
	sock = esp->links[PATMAT_RESULT_INT(res, 0)];
	mutex_unlock(&esp->mtx);

	esp->rx.sock = sock;
	esp->rx.len = PATMAT_RESULT_INT(res, 1);
	esp->rx.idx = 0;

	if(sock->type == SOCK_STREAM)
		return rx_datain_stream;

	esp->rx.dgram = kmalloc(esp->rx.len);

	if(esp->rx.dgram)
		return rx_datain_dgram;

	WARN("data loss: %s\n", strerror(errno));

	return rx_skipline;
}

static void rx_act_accept(esp_t *esp){
	void *res[6];
	sock_addr_inet_t remote;
	socket_t *client;


	(void)patmat_get_results(esp->rx.parser, res);

	remote.domain = AF_INET;
	remote.inet_data.port = (uint16_t)PATMAT_RESULT_INT(res, 5);
	remote.inet_data.addr = inet_addr(PATMAT_RESULT_STR(res, 4));

	mutex_lock(&esp->mtx);

	if(esp->tcp_server){
		client = socket_add_client_addr(esp->tcp_server, (sock_addr_t*)&remote, sizeof(sock_addr_inet_t), true);
		esp->links[PATMAT_RESULT_INT(res, 1)] = client;

		if(client == 0x0)
			FATAL("unable to add client (%s), link stays open\n", strerror(errno));
	}

	mutex_unlock(&esp->mtx);
}

static void rx_act_close(esp_t *esp){
	int link_id;
	void *res[1];


	(void)patmat_get_results(esp->rx.parser, res);

	link_id = PATMAT_RESULT_INT(res, 0);

	mutex_lock(&esp->mtx);

	if(esp->links[link_id] != 0x0)
		socket_unlink(esp->links[link_id]);

	esp->links[link_id] = 0x0;

	mutex_unlock(&esp->mtx);
}

static void rx_act_ip_info(esp_t *esp){
	void *res[2];
	char *info;
	inet_addr_t addr;


	(void)patmat_get_results(esp->rx.parser, res);

	info = PATMAT_RESULT_STR(res, 0);
	addr = inet_addr(PATMAT_RESULT_STR(res, 1));

	if(strcmp(info, "ip") == 0)				esp->cfg.ip = addr;
	else if(strcmp(info, "gateway") == 0)	esp->cfg.gateway = addr;
	else if(strcmp(info, "netmask") == 0)	esp->cfg.netmask = addr;

}

// tx interrupt handling
static void tx_hdlr(int_num_t num, void *payload){
	size_t n;
	esp_t *esp = (esp_t*)payload;
	tx_dgram_t *dgram;


	dgram = itask_query_payload(&esp->tx_queue, tx_complete);

	if(dgram == 0x0)
		return;

	/* output character */
	mutex_lock(&esp->mtx);
	n = esp->term->puts(dgram->s, dgram->len, false, esp->term->hw);
	mutex_unlock(&esp->mtx);

	dgram->s += n;
	dgram->len -= n;
}

static int tx_complete(void *payload){
	tx_dgram_t *dgram = (tx_dgram_t*)payload;
	term_itf_t *term = dgram->term;
	errno_t e;


	e = term->error ? term->error(term->hw) : 0;

	if(e != 0)
		return e;

	return (dgram->len == 0) ? 0 : -1;
}

// command handling
static int cmd(esp_t *esp, response_t resp, bool skip, char const *fmt, ...){
	int r = 0;
	char s[16];
	size_t len;
	va_list lst;


	if(skip)
		return 0;

	va_start(lst, fmt);

	DEBUG("cmd \"%s\"\n", fmt);

	for(char c=*fmt; c!=0; c=*(++fmt)){
		if(c == '%'){
			c = *(++fmt);

			if(c == 0)
				break;

			switch(c){
			case 'a':
				r |= puts(inet_ntoa(*va_arg(lst, inet_addr_t*)), esp);
				break;

			case 's':
				r |= puts(va_arg(lst, char*), esp);
				break;

			case 'x':
				len = va_arg(lst, size_t);
				r |= putsn(va_arg(lst, char*), len, esp);
				break;

			case 'u':
				snprintf(s, 16, "%u", va_arg(lst, int));
				r |= puts(s, esp);
				break;

			default:
				r |= putsn(&c, 1, esp);
			}
		}
		else
			r |= putsn(&c, 1, esp);
	}

	r |= putsn("\r\n", 2, esp);

	if(r != 0)
		goto_errno(end, E_IO);

	ksignal_wait(&esp->sig, &esp->mtx);

	if(esp->resp != resp){
		set_errno((esp->resp == RESP_BUSY) ? E_INUSE : E_IO);
		r = 1;
	}

	esp->resp = RESP_INVAL;

	mutex_unlock(&esp->mtx);

end:
	va_end(lst);

	return (r == 0 ? 0 : -1);
}

static void cmd_resp(esp_t *esp, ssize_t resp){
	mutex_lock(&esp->mtx);

	esp->resp = resp;
	ksignal_send(&esp->sig);

	mutex_unlock(&esp->mtx);
}

static int puts(char const *s, esp_t *esp){
	return putsn(s, strlen(s), esp);
}

static int putsn(char const *s, size_t n, esp_t *esp){
	tx_dgram_t dgram;


	dgram.s = s;
	dgram.len = n;
	dgram.term = esp->term;

	(void)itask_issue(&esp->tx_queue, &dgram, esp->term->tx_int);

	return dgram.len;
}

// utilities
static int get_link_id(esp_t *esp, socket_t *sock){
	for(uint8_t i=0; i<CONFIG_ESP8266_LINK_COUNT; i++){
		if(esp->links[i] == sock)
			return i;
	}

	return -1;
}
