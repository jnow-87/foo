/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/math.h>
#include <sys/string.h>
#include "dgram.h"


/* macros */
#define CTRLBYTE(brdg)			((brdg)->cfg->chunksize)
#define CHUNKSIZE(dgram, brdg)	(MIN((brdg)->cfg->chunksize, (dgram)->len - (dgram)->offset))

#ifdef CONFIG_BRIDGE_DEBUG_PROTOCOL
# define PROTO_DEBUG(dgram, fmt, ...)	DEBUG("%s: " fmt, dgram_state(dgram), ##__VA_ARGS__)
#else
# define PROTO_DEBUG(dgram, fmt, ...)	{}
#endif // CONFIG_BRIDGE_DEBUG


/* local/static prototypes */
static void dgram_init_data(bridge_dgram_t *dgram, void *data, uint8_t len, bridge_t *brdg);

static int rx(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t *byte);
static bridge_dgram_state_t rx_payload(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte);

static bridge_dgram_state_t tx(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_state_t next);
static bridge_dgram_state_t tx_payload(bridge_t *brdg, bridge_dgram_t *dgram);

static bridge_dgram_state_t hdrcmp(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, uint8_t ref, bridge_dgram_state_t next);

static bridge_dgram_state_t ack(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_state_t next);
static bridge_dgram_state_t nack(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_error_t ecode);
static int ackcmp(bridge_dgram_t *dgram, uint8_t byte, uint8_t ref);

static uint8_t checksum(uint8_t const *data, size_t n);

static bridge_dgram_state_t seterr(bridge_dgram_t *dgram, bridge_dgram_error_t ecode);

#ifdef CONFIG_BRIDGE_DEBUG_PROTOCOL
static char const *dgram_state(bridge_dgram_t *dgram);
static char const *dgram_error(bridge_dgram_t *dgram);
#endif // CONFIG_BRIDGE_DEBUG_PROTOCOL


/* global functions */
bridge_dgram_t *dgram_alloc_rx(bridge_t *brdg){
	bridge_dgram_t *dgram;


	dgram = kmalloc(sizeof(bridge_dgram_t));

	if(dgram == 0x0)
		return 0x0;

	dgram_init(dgram, BDT_READ, 0x0, 0, brdg);
	list_add_tail(brdg->rx_dgrams, dgram);

	return dgram;
}

bridge_dgram_t *dgram_alloc_tx(bridge_t *brdg, void const *data, uint8_t len){
	bridge_dgram_t *dgram;


	dgram = kmalloc(sizeof(bridge_dgram_t) + len);

	if(dgram == 0x0)
		return 0x0;

	dgram->data = (void*)dgram + sizeof(bridge_dgram_t);
	memcpy(dgram->data, data, len);

	dgram_init(dgram, BDT_WRITE, dgram->data, len, brdg);
	list_add_tail(brdg->tx_dgrams, dgram);

	return dgram;
}

void dgram_free(bridge_dgram_t *dgram, bridge_t *brdg){
	if(dgram->type == BDT_READ){
		list_rm(brdg->rx_dgrams, dgram);
		kfree(dgram->data);
	}
	else
		list_rm(brdg->tx_dgrams, dgram);

	kfree(dgram);
}

void dgram_init(bridge_dgram_t *dgram, bridge_dgram_type_t type, void *data, uint8_t len, bridge_t *brdg){
	memset(dgram, 0, sizeof(bridge_dgram_t));

	dgram->type = type;
	dgram->state = BDS_CTRL_BYTE;
	dgram->estate = BDS_ERROR;

	dgram_init_data(dgram, data, len, brdg);

	if(type == BDT_WRITE)
		dgram->checksum = checksum(data, len);
}

int dgram_init_retry(bridge_dgram_t *dgram){
	dgram->state = BDS_CTRL_BYTE;
	dgram->estate = BDS_ERROR;
	dgram->ecode = BDE_NONE;
	dgram->attempts++;

	return (dgram->attempts >= CONFIG_BRIDGE_RETRY_LIMIT) ? -1 : 0;
}

bridge_dgram_state_t dgram_read(bridge_dgram_t *dgram, bridge_t *brdg){
	uint8_t byte;
	uint8_t csum;


	/* read incoming byte if not in an acknowledge state */
	if(!BDS_ISACK(dgram->state)){
		if(rx(brdg, dgram, &byte) != 0)
			return seterr(dgram, BDE_RX);
	}

	/* perform state action and state change */
	switch(dgram->state){
	case BDS_CTRL_BYTE:
		return hdrcmp(brdg, dgram, byte, CTRLBYTE(brdg), BDS_CTRL_BYTE_ACK);

	case BDS_DATA_LEN:
		if(dgram->data == 0x0)
			dgram_init_data(dgram, kmalloc(byte), byte, brdg);

		if(dgram->data == 0x0)
			return nack(brdg, dgram, byte, BDE_NOMEM);

		return hdrcmp(brdg, dgram, byte, dgram->len, BDS_DATA_LEN_ACK);

	case BDS_CHECKSUM:
		dgram->checksum = byte;

		return ack(brdg, dgram, byte, BDS_CHECKSUM_ACK);

	case BDS_DATA:
		return rx_payload(brdg, dgram, byte);

	case BDS_DATA_ACK:
		if(dgram->chunksize)
			return BDS_DATA;

		csum = checksum(dgram->data, dgram->len);

		if(csum != dgram->checksum)
			seterr(dgram, BDE_CHECKSUM);

		return tx(brdg, dgram, csum, (csum != dgram->checksum) ? BDS_ERROR : BDS_VERIFY_ACK);

	case BDS_VERIFY_TX:
		return (ackcmp(dgram, byte, dgram->checksum) != 0) ? BDS_ERROR : BDS_COMPLETE;

	case BDS_CTRL_BYTE_ACK:	return BDS_DATA_LEN;
	case BDS_DATA_LEN_ACK:	return BDS_CHECKSUM;
	case BDS_CHECKSUM_ACK:	return BDS_DATA;
	case BDS_VERIFY_ACK:	return BDS_VERIFY_TX;

	default:				return BDS_ERROR;
	}
}

bridge_dgram_state_t dgram_write(bridge_dgram_t *dgram, bridge_t *brdg){
	uint8_t byte;


	/* read ack byte in acknowledge states */
	if(BDS_ISACK(dgram->state)){
		if(rx(brdg, dgram, &byte) != 0)
			return seterr(dgram, BDE_RX);
	}

	/* perform state action and state change */
	switch(dgram->state){
	case BDS_CTRL_BYTE:
		return tx(brdg, dgram, CTRLBYTE(brdg), BDS_CTRL_BYTE_TX);

	case BDS_CTRL_BYTE_ACK:
		if(ackcmp(dgram, byte, CTRLBYTE(brdg)) != 0)
			return BDS_ERROR;

		// fall through
	case BDS_DATA_LEN:
		return tx(brdg, dgram, dgram->len, BDS_DATA_LEN_TX);

	case BDS_DATA_LEN_ACK:
		if(ackcmp(dgram, byte, dgram->len) != 0)
			return BDS_ERROR;

		// fall through
	case BDS_CHECKSUM:
		return tx(brdg, dgram, dgram->checksum, BDS_CHECKSUM_TX);

	case BDS_CHECKSUM_ACK:
		if(ackcmp(dgram, byte, dgram->checksum) != 0)
			return BDS_ERROR;

		// fall through
	case BDS_DATA:
		return tx_payload(brdg, dgram);

	case BDS_DATA_ACK:
		if(ackcmp(dgram, byte, ((uint8_t*)dgram->data)[dgram->offset - 1]) != 0)
			return BDS_ERROR;

		if(dgram->chunksize)
			return tx_payload(brdg, dgram);

		return BDS_VERIFY_ACK;

	case BDS_VERIFY_ACK:
		if(byte != dgram->checksum)
			return nack(brdg, dgram, byte, BDE_CHECKSUM);

		return ack(brdg, dgram, byte, BDS_VERIFY_TX);

	case BDS_CTRL_BYTE_TX:	return BDS_CTRL_BYTE_ACK;
	case BDS_DATA_LEN_TX:	return BDS_DATA_LEN_ACK;
	case BDS_CHECKSUM_TX:	return BDS_CHECKSUM_ACK;
	case BDS_DATA_TX:		return BDS_DATA_ACK;
	case BDS_VERIFY_TX:		return BDS_COMPLETE;

	default:				return BDS_ERROR;
	}
}

errno_t dgram_errno(bridge_dgram_t *dgram){
	return (dgram->ecode == BDE_NOMEM) ? E_NOMEM : E_IO;
}


/* local functions */
static void dgram_init_data(bridge_dgram_t *dgram, void *data, uint8_t len, bridge_t *brdg){
	dgram->len = len;
	dgram->chunksize = CHUNKSIZE(dgram, brdg);
	dgram->data = data;
}

static int rx(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t *byte){
	while(brdg->ops.readb(byte, brdg->hw) != 0){
		if(errno != E_AGAIN)
			return -1;

		set_errno(E_OK);
	}

	PROTO_DEBUG(dgram, "read %#hhx/~%#hhx\n", *byte, ~(*byte));

	return 0;
}

static bridge_dgram_state_t rx_payload(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte){
	((uint8_t*)dgram->data)[dgram->offset] = byte;

	dgram->offset++;
	dgram->chunksize--;

	if(dgram->chunksize)
		return BDS_DATA;

	dgram->chunksize = CHUNKSIZE(dgram, brdg);

	return ack(brdg, dgram, byte, BDS_DATA_ACK);
}

static bridge_dgram_state_t tx(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_state_t next){
	PROTO_DEBUG(dgram, "write %#hhx/~%#hhx\n", byte, ~byte);

	if(brdg->ops.writeb(byte, brdg->hw) != 0)
		return seterr(dgram, BDE_TX);

	return next;
}

static bridge_dgram_state_t tx_payload(bridge_t *brdg, bridge_dgram_t *dgram){
	uint8_t byte;


	byte = ((uint8_t*)dgram->data)[dgram->offset];

	dgram->offset++;
	dgram->chunksize--;

	if(dgram->chunksize)
		return tx(brdg, dgram, byte, BDS_DATA);

	dgram->chunksize = CHUNKSIZE(dgram, brdg);

	return tx(brdg, dgram, byte, BDS_DATA_TX);
}

static bridge_dgram_state_t hdrcmp(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, uint8_t ref, bridge_dgram_state_t next){
	if(byte != ref){
		PROTO_DEBUG(dgram, "header byte mismatch, expected %#hhx\n", ref);

		return nack(brdg, dgram, byte, BDE_HDRBYTE);
	}

	return ack(brdg, dgram, byte, next);
}

static bridge_dgram_state_t ack(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_state_t next){
	if(next != BDS_ERROR)
		PROTO_DEBUG(dgram, "ack byte %#hhx\n", byte);

	return tx(brdg, dgram, ~byte, next);
}

static bridge_dgram_state_t nack(bridge_t *brdg, bridge_dgram_t *dgram, uint8_t byte, bridge_dgram_error_t ecode){
	seterr(dgram, ecode);
	PROTO_DEBUG(dgram, "nack byte %#hhx\n", byte);

	return ack(brdg, dgram, ~byte, BDS_ERROR);
}

static int ackcmp(bridge_dgram_t *dgram, uint8_t byte, uint8_t ref){
	if(((~byte) & 0xff) != ref){
		seterr(dgram, BDE_ACK);

		return -1;
	}

	PROTO_DEBUG(dgram, "ack byte ok\n");

	return 0;
}

static uint8_t checksum(uint8_t const *data, size_t n){
	uint8_t s;
	size_t i;


	s = 0;

	for(i=0; i<n; i++)
		s += ~data[i] + 1;

	return s;
}

static bridge_dgram_state_t seterr(bridge_dgram_t *dgram, bridge_dgram_error_t ecode){
	dgram->estate = dgram->state;
	dgram->ecode = ecode;

	PROTO_DEBUG(dgram, "%s\n", dgram_error(dgram));

	return BDS_ERROR;
}

#ifdef CONFIG_BRIDGE_DEBUG_PROTOCOL
static char const *dgram_state(bridge_dgram_t *dgram){
	return (char const *[]){
		"INVALID",
		"CTRL-BYTE",
		"DATA-LEN",
		"CHECKSUM",
		"DATA",
		"COMPLETE",
		"ERROR",
		"CTRL-BYTE-TX",
		"DATA-LEN-TX",
		"CHECKSUM-TX",
		"DATA-TX",
		"VERIFY-TX",
		"CTRL-BYTE-ACK",
		"DATA-LEN-ACK",
		"CHECKSUM-ACK",
		"DATA-ACK",
		"VERIFY-ACK",
	}[dgram->state >> 2];
}
#endif // CONFIG_BRIDGE_DEBUG_PROTOCOL

#ifdef CONFIG_BRIDGE_DEBUG_PROTOCOL
static char const *dgram_error(bridge_dgram_t *dgram){
	return (char const *[]){
		"none",
		"header byte mismatch",
		"checksum mismatch",
		"ack mismatch",
		"read failed",
		"write failed",
		"out of memory",
	}[dgram->ecode];
}
#endif // CONFIG_BRIDGE_DEBUG_PROTOCOL
