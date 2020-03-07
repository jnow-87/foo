/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <socket.h>
#include <sys/ioctl.h>
#include <sys/net.h>
#include <sys/inet.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <shell/cmd/test/test.h>


/* macros */
#define NET_DEV		"/dev/eth0"
#define ESSID		"dlink_home"
#define PASSWORD	"Wesd2xc3X"
#define IP			"192.168.0.24"
#define PORT		1234


/* local/static prototypes */
static int netdev_configure(void);
static int exec_server(sock_type_t type);
static int exec_client(sock_type_t type);


/* local functions */
/**
 *	\brief	test to verify the interrupt feature of gpio devices
 */
static int exec_server_udp(void){
	if(netdev_configure() != 0)
		return -1;

	return exec_server(SOCK_DGRAM);
}

test("socket-server-udp", exec_server_udp, "udp server");

static int exec_server_tcp(void){
	if(netdev_configure() != 0)
		return -1;

	return exec_server(SOCK_STREAM);
}

test("socket-server-tcp", exec_server_tcp, "tcp server");

static int exec_client_udp(void){
	if(netdev_configure() != 0)
		return -1;

	return exec_client(SOCK_DGRAM);
}

test("socket-client-udp", exec_client_udp, "udp client");

static int exec_client_tcp(void){
	if(netdev_configure() != 0)
		return -1;

	return exec_client(SOCK_STREAM);
}

test("socket-client-tcp", exec_client_tcp, "tcp client");

static int exec_netdev_configure(void){
	return netdev_configure();
}

test("netdev-configure", exec_netdev_configure, "configure netdev");

static int exec_udp(void){
	return exec_client(SOCK_DGRAM);
}

test("socket-udp", exec_udp, "udp send/recv");


static int netdev_configure(void){
	int fd,
		r;
	inetdev_cfg_t cfg;


	r = 0;
	fd = open(NET_DEV, O_RDWR);

	if(fd < 0){
		ERROR("opening net-dev \"%s\" \"%s\"\n", NET_DEV, strerror(errno), cfg);
		return -1;
	}

	cfg.mode = INET_CLIENT;

	cfg.dhcp = true;
	cfg.enc = ENC_WPA2_PSK;

	cfg.ip = inet_addr("192.168.0.1");
	cfg.gw = inet_addr("192.168.0.1");
	cfg.netmask = inet_addr("255.255.255.0");

	strcpy(cfg.ssid, ESSID);
	strcpy(cfg.password, PASSWORD);
	strcpy(cfg.hostname, "not-supported");

	r |= ioctl(fd, IOCTL_CFGWR, &cfg, sizeof(cfg));

	if(r != 0)
		ERROR("configuring net-dev \"%s\"\n", strerror(errno));

	r |= close(fd);

	return -r;
}

static int exec_server(sock_type_t type){
	int sock,
		client;
	char data[8];
	size_t addr_len;
	sock_addr_inet_t addr;


	sock = socket(AF_INET, type);

	if(sock < 0){
		ERROR("creating socket \"%s\"\n", strerror(errno));
		return -1;
	}

	addr.domain = AF_INET;
	addr.data.addr = INET_ADDR_ANY;
	addr.data.port = PORT;

	printf("start server on port %d\n", PORT);

	if(bind(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t)) != 0){
		ERROR("binding to address \"%s\" \"%s\"\n", inet_ntoa(addr.data.addr), strerror(errno));
		return -1;
	}

	if(type == SOCK_STREAM && listen(sock, 0) != 0){
		ERROR("listening \"%s\"\n", strerror(errno));
		return -1;
	}

	printf("waiting for client\n");

	addr_len = sizeof(sock_addr_inet_t);
	memset(&addr, 0x0, addr_len);

	while(1){
		write(1, ".", 1);
		client = accept(sock, (sock_addr_t*)&addr, &addr_len);

		if(client > 0)
			break;

		sleep(1000, 0);
	}

	printf("client socket %d %s on port %d\n", client, inet_ntoa(addr.data.addr), addr.data.port);

	data[7] = 0;
	printf("recv: %d %s\n", recv(sock, data, 7), data);
	printf("send: %d\n", send(sock, "server-test", 11));

	printf("close client: %d\n", close(client));
	printf("close server: %d\n", close(sock));

	return 0;
}

static int exec_client(sock_type_t type){
	int sock;
	ssize_t r;
	char data[8];
	sock_addr_inet_t addr;
	f_mode_t f_mode;


	sock = socket(AF_INET, type);

	if(sock < 0){
		ERROR("creating socket \"%s\"\n", strerror(errno));
		return -1;
	}

	printf("change file mode\n");

	if(fcntl(sock, F_MODE_GET, &f_mode, sizeof(f_mode_t)) != 0){
		ERROR("get file-mode \"%s\"\n", strerror(errno));
		return -1;
	}

	printf("file mode: %#x\n", f_mode);

	f_mode |= O_NONBLOCK;

	if(fcntl(sock, F_MODE_SET, &f_mode, sizeof(f_mode_t)) != 0){
		ERROR("set file-mode \"%s\"\n", strerror(errno));
		return -1;
	}

	printf("file mode: %#x\n", f_mode);

	addr.domain = AF_INET;
	addr.data.addr = inet_addr(IP);
	addr.data.port = PORT;

	if(connect(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t)) != 0){
		ERROR("connecting to %s on port %d\n", inet_ntoa(addr.data.addr), addr.data.port);
		return -1;
	}

	printf("send %d\n", send(sock, "client-test\n", 12));

	while(1){
		r = recv(sock, data, 7);

		if(r < 0)
			break;

		data[r] = 0;
		printf("recv %d \"%s\"\n", r, data);

		if(r == 0)
			sleep(1000, 0);
	}

	printf("close socket\n");
	printf("close: %d\n", close(sock));

	return 0;
}
