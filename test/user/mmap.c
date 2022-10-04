/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/loop.h>
#include <test/test.h>


/* macros */
#define DEVNAME	"loop0"
#define DEVICE	"/dev/" DEVNAME


/* local functions */
TEST(mmap){
	int r;
	int fd_rd,
		fd_wr;
	void *mem;
	char buf[8];
	loop_cfg_t cfg;


	r = 0;

	ASSERT_INT_NEQ(fd_rd = open(DEVICE, O_RDWR), -1);
	ASSERT_INT_NEQ(fd_wr = open(DEVICE, O_RDWR), -1);

	/* determine loop size */
	ASSERT_INT_EQ(ioctl(fd_rd, IOCTL_CFGRD, &cfg), 0);

	/* try mapping a too large block */
	r += TEST_PTR_EQ(mmap(fd_wr, cfg.size + 1), 0x0);
	r += TEST_INT_EQ(errno, E_LIMIT);
	reset_errno();

	/* write + read through mmaped block */
	ASSERT_PTR_NEQ(mem = mmap(fd_wr, cfg.size), 0x0);
	r += TEST_INT_EQ(write(fd_wr, "foobar", 6), 6);
	r += TEST_STRN_EQ(mem, "foobar", 6);

	/* write through mmapped block + read */
	strcpy(mem, "barfoo");
	r += TEST_INT_EQ(read(fd_rd, buf, 6), 6);
	r += TEST_STRN_EQ(buf, "barfoo", 6);

	r += TEST_INT_EQ(close(fd_rd), 0);
	r += TEST_INT_EQ(close(fd_wr), 0);

	return -r;
}
