/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ringbuf.h>
#include <sys/string.h>
#include <test/test.h>


/* macros */
#define INIT()		ringbuf_init(&ring, buf, 10)
#define READ(n)		ringbuf_read(&ring, readb, n)
#define WRITE(s)	({ typeof(s) _s = s; ringbuf_write(&ring, _s, strlen(_s)); })


/* static variables */
static ringbuf_t ring;
static char buf[10];
static char readb[10];


/* local functions */
TEST(ringbuf_read){
	int r = 0;


	/* normal read */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef"), 8);

	r |= TEST_INT_EQ(READ(2), 2);
	r |= TEST_STRN_EQ(readb, "de", 2);

	/* read more than data are available */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef"), 8);

	r |= TEST_INT_EQ(READ(100), 8);
	r |= TEST_STRN_EQ(readb, "deadbeef", 8);

	/* read with wrap around the buffer end */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef01"), 9);
	r |= TEST_INT_EQ(READ(4), 4);
	r |= TEST_STRN_EQ(readb, "dead", 4);

	r |= TEST_INT_EQ(WRITE("2345"), 4);
	r |= TEST_INT_EQ(READ(10), 9);
	r |= TEST_STRN_EQ(readb, "beef02345", 9);

	return -r;
}

TEST(ringbuf_write){
	int r = 0;


	/* normal write */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef"), 8);
	r |= TEST_STRN_EQ(ring.buf, "deadbeef", 8);

	/* write entire buffer */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef01"), 9);
	r |= TEST_STRN_EQ(ring.buf, "deadbeef0", 9);

	/* write with wrap around the buffer end */
	INIT();

	r |= TEST_INT_EQ(WRITE("deadbeef"), 8);
	r |= TEST_STRN_EQ(ring.buf, "deadbeef", 8);
	r |= TEST_INT_EQ(READ(2), 2);
	r |= TEST_INT_EQ(WRITE("0123"), 3);
	r |= TEST_STRN_EQ(ring.buf, "2eadbeef01", 10);

	return -r;
}

TEST(ringbuf_contains_left){
	int r = 0;


	/* empty */
	INIT();
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 0);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 9);

	/* one used */
	INIT();
	WRITE("dead");
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 4);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 5);

	/* all used */
	INIT();
	WRITE("deadbeef0");
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 9);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 0);

	/* wrap around */
	INIT();
	WRITE("deadbeef0");
	READ(4);
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 5);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 4);

	WRITE("123");
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 8);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 1);

	READ(3);
	r |= TEST_INT_EQ(ringbuf_contains(&ring), 5);
	r |= TEST_INT_EQ(ringbuf_left(&ring), 4);

	return -r;
}

TEST(ringbuf_empty){
	int r = 0;


	INIT();

	/* initially empty */
	r |= TEST_INT_EQ(ringbuf_empty(&ring), true);

	/* non-empty */
	WRITE("dead");
	r |= TEST_INT_EQ(ringbuf_empty(&ring), false);

	/* still not empty */
	READ(3);
	r |= TEST_INT_EQ(ringbuf_empty(&ring), false);

	/* empty again */
	READ(1);
	r |= TEST_INT_EQ(ringbuf_empty(&ring), true);

	return -r;
}

TEST(ringbuf_full){
	int r = 0;


	INIT();

	r |= TEST_INT_EQ(ringbuf_full(&ring), false);

	WRITE("deadbeef");
	r |= TEST_INT_EQ(ringbuf_full(&ring), false);

	WRITE("1");
	r |= TEST_INT_EQ(ringbuf_full(&ring), true);

	return -r;
}
