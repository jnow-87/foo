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
static int tc_ringbuf_read(int log){
	size_t i;
	int n;


	n = 0;

	/* normal read */
	INIT();

	n += check_int(log, WRITE("deadbeef"), 8);

	n += check_strn(log, i = READ(2), readb, "de", 2);
	n += check_int(log, i, 2);

	/* read more than data are available */
	INIT();

	n += check_int(log, WRITE("deadbeef"), 8);

	n += check_strn(log, i = READ(100), readb, "deadbeef", 8);
	n += check_int(log, i, 8);

	/* read with wrap around the buffer end */
	INIT();

	n += check_int(log, WRITE("deadbeef01"), 9);
	n += check_strn(log, i = READ(4), readb, "dead", 4);
	n += check_int(log, i, 4);

	n += check_int(log, WRITE("2345"), 4);
	n += check_strn(log, i = READ(10), readb, "beef02345", 9);
	n += check_int(log, i, 9);

	return -n;
}

test_case(tc_ringbuf_read, "ringbuf_read");

static int tc_ringbuf_write(int log){
	int n;
	size_t i;


	n = 0;

	/* normal write */
	INIT();

	n += check_strn(log, i = WRITE("deadbeef"), ring.data, "deadbeef", 8);
	n += check_int(log, i, 8);

	/* write entire buffer */
	INIT();

	n += check_strn(log, i = WRITE("deadbeef01"), ring.data, "deadbeef0", 9);
	n += check_int(log, i, 9);

	/* write with wrap around the buffer end */
	INIT();

	n += check_strn(log, i = WRITE("deadbeef"), ring.data, "deadbeef", 8);
	n += check_int(log, i, 8);
	n += check_int(log, i = READ(2), 2);
	n += check_strn(log, i = WRITE("0123"), ring.data, "2eadbeef01", 10);
	n += check_int(log, i, 3);

	return -n;
}

test_case(tc_ringbuf_write, "ringbuf_write");

static int tc_ringbuf_contains_left(int log){
	int n;


	n = 0;

	/* empty */
	INIT();
	n += check_int(log, ringbuf_contains(&ring), 0);
	n += check_int(log, ringbuf_left(&ring), 9);

	/* one used */
	INIT();
	WRITE("dead");
	n += check_int(log, ringbuf_contains(&ring), 4);
	n += check_int(log, ringbuf_left(&ring), 5);

	/* all used */
	INIT();
	WRITE("deadbeef0");
	n += check_int(log, ringbuf_contains(&ring), 9);
	n += check_int(log, ringbuf_left(&ring), 0);

	/* wrap around */
	INIT();
	WRITE("deadbeef0");
	READ(4);
	n += check_int(log, ringbuf_contains(&ring), 5);
	n += check_int(log, ringbuf_left(&ring), 4);

	WRITE("123");
	n += check_int(log, ringbuf_contains(&ring), 8);
	n += check_int(log, ringbuf_left(&ring), 1);

	READ(3);
	n += check_int(log, ringbuf_contains(&ring), 5);
	n += check_int(log, ringbuf_left(&ring), 4);

	return -n;
}

test_case(tc_ringbuf_contains_left, "ringbuf_contains_left");

static int tc_ringbuf_empty(int log){
	int n;


	n = 0;

	INIT();

	/* initially empty */
	n += check_int(log, ringbuf_empty(&ring), true);

	/* non-empty */
	WRITE("dead");
	n += check_int(log, ringbuf_empty(&ring), false);

	/* still not empty */
	READ(3);
	n += check_int(log, ringbuf_empty(&ring), false);

	/* empty again */
	READ(1);
	n += check_int(log, ringbuf_empty(&ring), true);

	return -n;
}

test_case(tc_ringbuf_empty, "ringbuf_empty");

static int tc_ringbuf_full(int log){
	int n;


	n = 0;

	INIT();

	n += check_int(log, ringbuf_full(&ring), false);

	WRITE("deadbeef");
	n += check_int(log, ringbuf_full(&ring), false);

	WRITE("1");
	n += check_int(log, ringbuf_full(&ring), true);

	return -n;
}

test_case(tc_ringbuf_full, "ringbuf_full");
