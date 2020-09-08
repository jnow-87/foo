/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ringbuf.h>
#include <sys/string.h>
#include <testing/testcase.h>


/* macros */
#define INIT()		ringbuf_init(&ring, data, 10)
#define READ(n)		ringbuf_read(&ring, readb, n)
#define WRITE(s)	ringbuf_write(&ring, s, strlen(s))


/* static variables */
ringbuf_t ring;
char data[10];
char readb[10];


/* local functions */
TEST(ringbuf_read, "ringbuf read"){
	size_t i;
	int n;


	n = 0;

	/* normal read */
	INIT();

	n += CHECK_INT(WRITE("deadbeef"), 8);

	n += CHECK_STRN(i = READ(2), readb, "de", 2);
	n += CHECK_INT(i, 2);

	/* read more than data are available */
	INIT();

	n += CHECK_INT(WRITE("deadbeef"), 8);

	n += CHECK_STRN(i = READ(100), readb, "deadbeef", 8);
	n += CHECK_INT(i, 8);

	/* read with wrap around the buffer end */
	INIT();

	n += CHECK_INT(WRITE("deadbeef01"), 9);
	n += CHECK_STRN(i = READ(4), readb, "dead", 4);
	n += CHECK_INT(i, 4);

	n += CHECK_INT(WRITE("2345"), 4);
	n += CHECK_STRN(i = READ(10), readb, "beef02345", 9);
	n += CHECK_INT(i, 9);

	return -n;
}

TEST(ringbuf_write, "ringbuf write"){
	int n;
	size_t i;


	n = 0;

	/* normal write */
	INIT();

	n += CHECK_STRN(i = WRITE("deadbeef"), ring.data, "deadbeef", 8);
	n += CHECK_INT(i, 8);

	/* write entire buffer */
	INIT();

	n += CHECK_STRN(i = WRITE("deadbeef01"), ring.data, "deadbeef0", 9);
	n += CHECK_INT(i, 9);

	/* write with wrap around the buffer end */
	INIT();

	n += CHECK_STRN(i = WRITE("deadbeef"), ring.data, "deadbeef", 8);
	n += CHECK_INT(i, 8);
	n += CHECK_INT(i = READ(2), 2);
	n += CHECK_STRN(i = WRITE("0123"), ring.data, "2eadbeef01", 10);
	n += CHECK_INT(i, 3);

	return -n;
}

TEST(ringbuf_contains_left, "ringbuf contains left"){
	int n;


	n = 0;

	/* empty */
	INIT();
	n += CHECK_INT(ringbuf_contains(&ring), 0);
	n += CHECK_INT(ringbuf_left(&ring), 9);

	/* one used */
	INIT();
	WRITE("dead");
	n += CHECK_INT(ringbuf_contains(&ring), 4);
	n += CHECK_INT(ringbuf_left(&ring), 5);

	/* all used */
	INIT();
	WRITE("deadbeef0");
	n += CHECK_INT(ringbuf_contains(&ring), 9);
	n += CHECK_INT(ringbuf_left(&ring), 0);

	/* wrap around */
	INIT();
	WRITE("deadbeef0");
	READ(4);
	n += CHECK_INT(ringbuf_contains(&ring), 5);
	n += CHECK_INT(ringbuf_left(&ring), 4);

	WRITE("123");
	n += CHECK_INT(ringbuf_contains(&ring), 8);
	n += CHECK_INT(ringbuf_left(&ring), 1);

	READ(3);
	n += CHECK_INT(ringbuf_contains(&ring), 5);
	n += CHECK_INT(ringbuf_left(&ring), 4);

	return -n;
}

TEST(ringbuf_empty, "ringbuf empty"){
	int n;


	n = 0;

	INIT();

	/* initially empty */
	n += CHECK_INT(ringbuf_empty(&ring), true);

	/* non-empty */
	WRITE("dead");
	n += CHECK_INT(ringbuf_empty(&ring), false);

	/* still not empty */
	READ(3);
	n += CHECK_INT(ringbuf_empty(&ring), false);

	/* empty again */
	READ(1);
	n += CHECK_INT(ringbuf_empty(&ring), true);

	return -n;
}

TEST(ringbuf_full, "ringbuf full"){
	int n;


	n = 0;

	INIT();

	n += CHECK_INT(ringbuf_full(&ring), false);

	WRITE("deadbeef");
	n += CHECK_INT(ringbuf_full(&ring), false);

	WRITE("1");
	n += CHECK_INT(ringbuf_full(&ring), true);

	return -n;
}
