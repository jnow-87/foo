/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/term.h>
#include <test/test.h>


/* macros */
#define DEVICE	"/dev/tty-loop0"


/* local/static prototypes */
static int test(int fd, char const *in, char const *out);
static int configure(int fd, term_iflags_t iflags, term_oflags_t oflags, term_lflags_t lflags);



/* local functions */
/**
 * \brief	test to verify terminal iflags
 */
TEST(tty_iflags){
	int fd;
	int r;


	r = 0;

	ASSERT_INT_NEQ(fd = open(DEVICE, O_RDWR), -1);

	// TIFL_CRNL
	ASSERT_INT_EQ(configure(fd, TIFL_CRNL, 0, 0), 0);
	r += test(fd, "\r", "\n");
	r += test(fd, "a\rb\rc", "a\nb\nc");

	// TIFL_NLCR
	ASSERT_INT_EQ(configure(fd, TIFL_NLCR, 0, 0), 0);
	r += test(fd, "\n", "\r");
	r += test(fd, "a\nb\nc", "a\rb\rc");

	r += TEST_INT_EQ(close(fd), 0);

	return -r;
}

/**
 * \brief	test to verify terminal oflags
 */
TEST(tty_oflags){
	int fd;
	int r;


	r = 0;

	ASSERT_INT_NEQ(fd = open(DEVICE, O_RDWR), -1);

	// TOFL_CRNL
	ASSERT_INT_EQ(configure(fd, 0, TOFL_CRNL, 0), 0);
	r += test(fd, "\r", "\n");
	r += test(fd, "a\rb\rc", "a\nb\nc");

	// TOFL_NLCR
	ASSERT_INT_EQ(configure(fd, 0, TOFL_NLCR, 0), 0);
	r += test(fd, "\n", "\r\n");
	r += test(fd, "a\nb\nc", "a\r\nb\r\nc");

	r += TEST_INT_EQ(close(fd), 0);

	return -r;
}

/**
 * \brief	test to verify terminal lflags
 */
TEST(tty_lflags){
	int fd;
	int r;
	char buf[4];


	r = 0;

	ASSERT_INT_NEQ(fd = open(DEVICE, O_RDWR), -1);

	// TLFL_ECHO
	ASSERT_INT_EQ(configure(fd, 0, 0, TLFL_ECHO), 0);
	r += test(fd, "foo", "foo");
	r += TEST_INT_EQ(configure(fd, 0, 0, 0), 0);
	r += TEST_INT_EQ(read(fd, buf, 4), 3);
	r += TEST_STRN_EQ(buf, "foo", 3);

	r += TEST_INT_EQ(close(fd), 0);

	return -r;
}

static int test(int fd, char const *in, char const *out){
	int r;
	size_t ilen,
		   olen;
	char buf[strlen(out)];


	r = 0;
	ilen = strlen(in);
	olen = strlen(out);

	r += TEST_INT_EQ(write(fd, (void*)in, ilen), ilen);
	r += TEST_INT_EQ(read(fd, buf, olen), olen);
	r += TEST_STRN_EQ(buf, out, olen);

	return -r;
}

static int configure(int fd, term_iflags_t iflags, term_oflags_t oflags, term_lflags_t lflags){
	term_cfg_t cfg;


	if(ioctl(fd, IOCTL_CFGRD, &cfg) != 0)
		return -1;

	cfg.iflags = iflags;
	cfg.oflags = oflags;
	cfg.lflags = lflags;

	if(ioctl(fd, IOCTL_CFGWR, &cfg) != 0)
		return -1;

	return 0;
}
