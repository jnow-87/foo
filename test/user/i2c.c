/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/escape.h>
#include <test/test.h>


/* macros */
#define I2C_DEV		"/dev/i2c0"


/* types */
typedef enum{
	MASTER = 0x1,
	SLAVE = 0x2,
	MASTER_RD = 0x4,
	MASTER_WR = 0x8,
} test_type_t;


/* local/static prototypes */
static int tests(test_type_t type);

static int test_err(test_type_t type, int fd);
static int test_rw(test_type_t type, int fd, size_t tx, size_t tx_exp, size_t rx, size_t rx_exp);


/* local functions */
/**
 * \brief	Verify the i2c protocol implementation.
 * 			This test has to be executed on the master device after starting
 * 			the slave device test.
 */
TEST_LONG(i2c_master, "i2c master half test"){
	return tests(MASTER);
}

/**
 * \brief	Verify the i2c protocol implementation.
 * 			This test has to be executed on the slave device before starting
 * 			the master device test.
 */
TEST_LONG(i2c_slave, "i2c slave half test"){
	return tests(SLAVE);
}

static int tests(test_type_t type){
	int r;
	int fd;


	r = 0;

	ASSERT_INT_NEQ(fd = open(I2C_DEV, O_RDWR), -1);

	/* read/write of different length */
	r += test_rw(type | MASTER_RD, fd, 1, 1, 1, 1);
	r += test_rw(type | MASTER_WR, fd, 1, 1, 1, 1);
	r += test_rw(type | MASTER_RD, fd, 4, 4, 4, 4);
	r += test_rw(type | MASTER_WR, fd, 4, 4, 4, 4);
	r += test_rw(type | MASTER_RD, fd, 5, 5, 5, 5);
	r += test_rw(type | MASTER_WR, fd, 5, 5, 5, 5);

	/* read/write with deviating tx and rx length */
	r += test_rw(type | MASTER_RD, fd, 5, 3, 3, 3);
	r += test_rw(type | MASTER_WR, fd, 5, 3, 3, 3);
	r += test_rw(type | MASTER_RD, fd, 3, 3, 5, 5);
	r += test_rw(type | MASTER_WR, fd, 3, 3, 5, 3);

	/* error cases: no slave connection */
	r += test_err(type | MASTER_RD, fd);
	r += test_err(type | MASTER_WR, fd);

	close(fd);

	return -r;
}

static int test_err(test_type_t type, int fd){
	char data[4] = { 0 };
	int r;


	if(type & SLAVE)
		return 0;

	r = 0;

	if(type & MASTER_RD)	r += TEST_INT_EQ(read(fd, data, 4), -1);
	else					r += TEST_INT_EQ(write(fd, data, 4), -1);

	r += TEST_INT_EQ(errno, E_NOCONN);

	return r;
}

static int test_rw(test_type_t type, int fd, size_t tx, size_t tx_exp, size_t rx, size_t rx_exp){
	int r;
	char tx_data[] = "deadbeef",
		 rx_data[8] = { 0 };


	r = 0;

	ASSERT_INT_EQ(tx <= 8, true);
	ASSERT_INT_EQ(rx <= 8, true);

	if(type & MASTER)
		msleep(50);

	if((type == (MASTER | MASTER_RD)) || (type == (SLAVE | MASTER_WR))){
		// expect to recv 0xff on master if more data are requested then sent
		if(tx_exp < rx_exp && type & MASTER)
			memset(tx_data + tx_exp, 0xff, rx_exp - tx_exp);

		r += TEST_INT_EQ(read(fd, rx_data, rx), rx_exp);
		r += TEST_STRN_EQ(rx_data, tx_data, rx_exp);
	}
	else{
		r += TEST_INT_EQ(write(fd, tx_data, tx), tx_exp);
	}

	return r;
}
