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
#define DS_ACK_FLAG				0x1
#define DS_TX_FLAG				0x2

#define DS_BASE(num)			(((num) << 2))
#define DS_TX(num)				(((num) << 2) | DS_TX_FLAG)
#define DS_ACK(num)				(((num) << 2) | DS_ACK_FLAG)

#define DS_ISTX					((state) & DS_TX_FLAG)
#define DS_ISACK(state)			((state) & DS_ACK_FLAG)


/* incomplete types */
struct bridge_t;


/* types */
typedef enum{
	DT_READ = 0,
	DT_WRITE,
} bridge_dgram_type_t;

typedef enum{
	DS_CTRL_BYTE = DS_BASE(1),
	DS_DATA_LEN = DS_BASE(2),
	DS_CHECKSUM = DS_BASE(3),
	DS_DATA = DS_BASE(4),

	DS_COMPLETE = DS_BASE(5),
	DS_ERROR = DS_BASE(6),

	DS_CTRL_BYTE_TX = DS_TX(7),
	DS_DATA_LEN_TX = DS_TX(8),
	DS_CHECKSUM_TX = DS_TX(9),
	DS_DATA_TX = DS_TX(10),
	DS_VERIFY_TX = DS_TX(11),

	DS_CTRL_BYTE_ACK = DS_ACK(12),
	DS_DATA_LEN_ACK = DS_ACK(13),
	DS_CHECKSUM_ACK = DS_ACK(14),
	DS_DATA_ACK = DS_ACK(15),
	DS_VERIFY_ACK = DS_ACK(16),
} bridge_dgram_state_t;

typedef enum{
	DE_NONE = 0,
	DE_HDRBYTE,		/**< header byte mismatch */
	DE_CHECKSUM,	/**< checksum mismatch */
	DE_ACK,			/**< acknowledged byte mismatch */
	DE_RX,			/**< read failed */
	DE_TX,			/**< write failed */
	DE_NOMEM,		/**< memory allocation error */
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
	void *data;

	uint8_t attempts;
	bridge_dgram_state_t estate;
	bridge_dgram_error_t ecode;
} bridge_dgram_t;

typedef struct{
	uint8_t id;
	uint8_t chunksize;

	uint8_t rx_int,
			tx_int;

	uint8_t hw_cfg[];
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

	errno_t errno;
} bridge_t;


/* prototypes */
bridge_t *bridge_create(bridge_cfg_t *cfg, bridge_ops_t *ops, void *hw);
void bridge_destroy(bridge_t *brdg);

int16_t bridge_read(bridge_t *brdg, void *data, uint8_t n);
int16_t bridge_write(bridge_t *brdg, void const *data, uint8_t n);

errno_t dgram_errno(bridge_dgram_t *dgram);


#endif // DRIVER_BRIDGE_H
