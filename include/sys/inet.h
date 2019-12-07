/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_INET_H
#define SYS_INET_H


#include <sys/types.h>
#include <sys/net.h>


/* macros */
#define INET_ADDR_ANY	0


/* types */
typedef uint32_t inet_addr_t;

typedef enum{
	INET_AP = 1,
	INET_CLIENT,
} inetdev_mode_t;

typedef enum{
	ENC_OPEN = 0,
	ENC_WPA_PSK,
	ENC_WPA2_PSK,
	ENC_WPA_WPA2_PSK,
} inet_enc_t;

typedef struct{
	inetdev_mode_t mode;

	char ssid[16],
		 hostname[16];

	bool dhcp;
	inet_addr_t ip,
				gw,
				netmask;

	inet_enc_t enc;
	char password[16];
} inetdev_cfg_t;

typedef struct{
	inet_addr_t addr;
	uint16_t port;
} inet_data_t;

typedef struct{
	net_family_t domain;
	inet_data_t data;
} sock_addr_inet_t;


/* prototypes */
inet_addr_t inet_addr(char *addr);
char *inet_ntoa(inet_addr_t addr);
char *inet_ntoa_r(inet_addr_t addr, char *s, size_t len);


#endif // SYS_INET_H
