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
#define INIT()		ringbuf_init(&ring, data, 10)
#define READ(n)		ringbuf_read(&ring, readb, n)
#define WRITE(s)	ringbuf_write(&ring, s, strlen(s))


/* static variables */
static ringbuf_t ring;
static char data[10];
static char readb[10];


/* local functions */
TEST(ringbuf_read){
	int n;


	n = 0;

	/* normal read */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_STRN_EQ(readb, "de", 2);

	/* read more than data are available */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(READ(100), 8);
	n += TEST_STRN_EQ(readb, "deadbeef", 8);

	/* read with wrap around the buffer end */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef01"), 9);
	n += TEST_INT_EQ(READ(4), 4);
	n += TEST_STRN_EQ(readb, "dead", 4);

	n += TEST_INT_EQ(WRITE("2345"), 4);
	n += TEST_INT_EQ(READ(10), 9);
	n += TEST_STRN_EQ(readb, "beef02345", 9);

	return -n;
}

TEST(ringbuf_write){
	int n;


	n = 0;

	/* normal write */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);
	n += TEST_STRN_EQ(ring.data, "deadbeef", 8);

	/* write entire buffer */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef01"), 9);
	n += TEST_STRN_EQ(ring.data, "deadbeef0", 9);

	/* write with wrap around the buffer end */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);
	n += TEST_STRN_EQ(ring.data, "deadbeef", 8);
	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_INT_EQ(WRITE("0123"), 3);
	n += TEST_STRN_EQ(ring.data, "2eadbeef01", 10);

	return -n;
}

TEST(ringbuf_contains_left){
	int n;


	n = 0;

	/* empty */
	INIT();
	n += TEST_INT_EQ(ringbuf_contains(&ring), 0);
	n += TEST_INT_EQ(ringbuf_left(&ring), 9);

	/* one used */
	INIT();
	WRITE("dead");
	n += TEST_INT_EQ(ringbuf_contains(&ring), 4);
	n += TEST_INT_EQ(ringbuf_left(&ring), 5);

	/* all used */
	INIT();
	WRITE("deadbeef0");
	n += TEST_INT_EQ(ringbuf_contains(&ring), 9);
	n += TEST_INT_EQ(ringbuf_left(&ring), 0);

	/* wrap around */
	INIT();
	WRITE("deadbeef0");
	READ(4);
	n += TEST_INT_EQ(ringbuf_contains(&ring), 5);
	n += TEST_INT_EQ(ringbuf_left(&ring), 4);

	WRITE("123");
	n += TEST_INT_EQ(ringbuf_contains(&ring), 8);
	n += TEST_INT_EQ(ringbuf_left(&ring), 1);

	READ(3);
	n += TEST_INT_EQ(ringbuf_contains(&ring), 5);
	n += TEST_INT_EQ(ringbuf_left(&ring), 4);

	return -n;
}

TEST(ringbuf_empty){
	int n;


	n = 0;

	INIT();

	/* initially empty */
	n += TEST_INT_EQ(ringbuf_empty(&ring), true);

	/* non-empty */
	WRITE("dead");
	n += TEST_INT_EQ(ringbuf_empty(&ring), false);

	/* still not empty */
	READ(3);
	n += TEST_INT_EQ(ringbuf_empty(&ring), false);

	/* empty again */
	READ(1);
	n += TEST_INT_EQ(ringbuf_empty(&ring), true);

	return -n;
}

TEST(ringbuf_full){
	int n;


	n = 0;

	INIT();

	n += TEST_INT_EQ(ringbuf_full(&ring), false);

	WRITE("deadbeef");
	n += TEST_INT_EQ(ringbuf_full(&ring), false);

	WRITE("1");
	n += TEST_INT_EQ(ringbuf_full(&ring), true);

	return -n;
}
