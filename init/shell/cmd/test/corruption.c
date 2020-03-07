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


/* types */
typedef struct{
	inetdev_mode_t mode;

	char *ssid,
		 *hostname;

	bool dhcp;
	inet_addr_t ip,
				gw,
				netmask;

	inet_enc_t enc;
	char *password;
} mod_inetdev_cfg_t;


/* local/static prototypes */
static int netdev_configure(void);
static int corruption0(sock_type_t type);
static int corruption1(sock_type_t type);



/* local functions */
/**
 *	\brief	test to verify the interrupt feature of gpio devices
 */
static int exec_corruption0(void){
	if(netdev_configure() != 0)
		return -1;

	return corruption0(SOCK_DGRAM);
}

test("corruption0", exec_corruption0, "corruption0");

static int exec_corruption1(void){
	if(netdev_configure() != 0)
		return -1;

	return corruption1(SOCK_DGRAM);
}

test("corruption1", exec_corruption1, "corruption1");


static int netdev_configure(void){
	int fd;
	mod_inetdev_cfg_t cfg;


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

	cfg.ssid = ESSID;
	cfg.hostname = "not-supported";
	cfg.password = PASSWORD;

	if(ioctl(fd, IOCTL_CFGWR, &cfg, sizeof(cfg)) != 0){
		ERROR("configuring net-dev \"%s\"\n", strerror(errno));
		close(fd);

		return -1;
	}

	close(fd);

	return 0;
}

static int corruption0(sock_type_t type){
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

static int corruption1(sock_type_t type){
	int sock;
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

	f_mode |= O_NONBLOCK;

	if(fcntl(sock, F_MODE_SET, &f_mode, sizeof(f_mode_t)) != 0){
		ERROR("set file-mode \"%s\"\n", strerror(errno));
		return -1;
	}

	printf("close: %d\n", close(sock));

	return 0;
}
