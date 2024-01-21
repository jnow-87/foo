/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_BRIDGE_H
#define DRIVER_BRIDGE_H


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>


/* macros */
#define BDS_ACK_FLAG			0x1
#define BDS_TX_FLAG				0x2

#define BDS_BASE(num)			(((num) << 2))
#define BDS_TX(num)				(((num) << 2) | BDS_TX_FLAG)
#define BDS_ACK(num)			(((num) << 2) | BDS_ACK_FLAG)

#define BDS_ISTX				((state) & BDS_TX_FLAG)
#define BDS_ISACK(state)		((state) & BDS_ACK_FLAG)


/* incomplete types */
struct bridge_t;


/* types */
typedef enum{
	BDT_READ = 0,
	BDT_WRITE,
} bridge_dgram_type_t;

typedef enum{
	BDS_CTRL_BYTE = BDS_BASE(1),
	BDS_DATA_LEN = BDS_BASE(2),
	BDS_CHECKSUM = BDS_BASE(3),
	BDS_DATA = BDS_BASE(4),

	BDS_COMPLETE = BDS_BASE(5),
	BDS_ERROR = BDS_BASE(6),

	BDS_CTRL_BYTE_TX = BDS_TX(7),
	BDS_DATA_LEN_TX = BDS_TX(8),
	BDS_CHECKSUM_TX = BDS_TX(9),
	BDS_DATA_TX = BDS_TX(10),
	BDS_VERIFY_TX = BDS_TX(11),

	BDS_CTRL_BYTE_ACK = BDS_ACK(12),
	BDS_DATA_LEN_ACK = BDS_ACK(13),
	BDS_CHECKSUM_ACK = BDS_ACK(14),
	BDS_DATA_ACK = BDS_ACK(15),
	BDS_VERIFY_ACK = BDS_ACK(16),
} bridge_dgram_state_t;

typedef enum{
	BDE_NONE = 0,
	BDE_HDRBYTE,	/**< header byte mismatch */
	BDE_CHECKSUM,	/**< checksum mismatch */
	BDE_ACK,		/**< acknowledged byte mismatch */
	BDE_RX,			/**< read failed */
	BDE_TX,			/**< write failed */
	BDE_NOMEM,		/**< memory allocation error */
} bridge_dgram_error_t;

typedef struct bridge_dgram_t{
	struct bridge_dgram_t *prev,
						  *next;

	bridge_dgram_type_t type;
	bridge_dgram_state_t state;

	uint8_t checksum;
	uint8_t len,
			offset,
			chunksize;
	void *buf;

	uint8_t attempts;
	bridge_dgram_state_t estate;
	bridge_dgram_error_t errnum;
} bridge_dgram_t;

typedef struct{
	uint8_t id;
	uint8_t chunksize;

	uint8_t rx_int,
			tx_int;

	uint8_t hwcfg[];
} bridge_cfg_t;

typedef struct{
	/**
	 * \brief	read a byte from hardware
	 *
	 * \param	b	pointer to read the byte to
	 * \param	hw	pointer to the underlying hardware
	 *
	 * \return	0 on success, any other value is treated as an error
	 */
	int (*readb)(uint8_t *b, void *hw);

	/**
	 * \brief	write a byte to hardware
	 *
	 * \param	b	byte to write
	 * \param	hw	pointer to the underlying hardware
	 *
	 * \return	0 on success, any other value is treated as an error
	 */
	int (*writeb)(uint8_t b, void *hw);
} bridge_ops_t;

typedef struct bridge_t{
	struct bridge_t *prev,
					*next;

	bridge_cfg_t *cfg;
	bridge_ops_t ops;
	void *hw;
	struct bridge_t *peer;

	mutex_t mtx;
	bridge_dgram_t *rx_dgrams,
				   *tx_dgrams;

	errno_t errnum;
} bridge_t;


/* prototypes */
bridge_t *bridge_create(bridge_ops_t *ops, bridge_cfg_t *cfg, void *hw);
void bridge_destroy(bridge_t *brdg);

int16_t bridge_read(bridge_t *brdg, void *buf, uint8_t n);
int16_t bridge_write(bridge_t *brdg, void *buf, uint8_t n);

errno_t dgram_errno(bridge_dgram_t *dgram);


#endif // DRIVER_BRIDGE_H
