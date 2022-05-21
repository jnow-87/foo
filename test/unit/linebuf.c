/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/linebuf.h>
#include <sys/string.h>
#include <test/test.h>


/* macros */
#define INIT()		linebuf_init(&buf, data, 10, 0)
#define READ(n)		linebuf_read(&buf, readb, n)
#define PEEK(n)		linebuf_peek(&buf, readb, n)
#define WRITE(s)	linebuf_write(&buf, s, strlen(s))


/* static variables */
static linebuf_t buf;
static char data[10];
static char readb[10];


/* local functions */
TEST(linebuf_reset){
	int n;


	n = 0;

	/* no change to empty buffer */
	INIT();

	n += TEST_INT_EQ(linebuf_contains(&buf), 0);
	n += TEST_INT_EQ(linebuf_left(&buf), 10);

	linebuf_reset(&buf);
	n += TEST_INT_EQ(linebuf_contains(&buf), 0);
	n += TEST_INT_EQ(linebuf_left(&buf), 10);

	/* reset after write */
	INIT();

	n += TEST_INT_EQ(WRITE("dead"), 4);
	n += TEST_INT_EQ(linebuf_contains(&buf), 4);
	n += TEST_INT_EQ(linebuf_left(&buf), 6);

	linebuf_reset(&buf);
	n += TEST_INT_EQ(linebuf_contains(&buf), 0);
	n += TEST_INT_EQ(linebuf_left(&buf), 10);

	/* reset after write + read */
	INIT();

	n += TEST_INT_EQ(WRITE("dead"), 4);
	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_INT_EQ(linebuf_contains(&buf), 2);
	n += TEST_INT_EQ(linebuf_left(&buf), 6);

	linebuf_reset(&buf);
	n += TEST_INT_EQ(linebuf_contains(&buf), 0);
	n += TEST_INT_EQ(linebuf_left(&buf), 10);

	return -n;
}

TEST(linebuf_read){
	int n;


	n = 0;

	/* normal read */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_STRN_EQ(readb, "de", 2);
	n += TEST_INT_EQ(READ(6), 6);
	n += TEST_STRN_EQ(readb, "adbeef", 6);

	/* read empty buffer */
	INIT();

	n += TEST_INT_EQ(READ(2), 0);

	/* read more than data are available */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(READ(100), 8);
	n += TEST_STRN_EQ(readb, "deadbeef", 8);

	return -n;
}

TEST(linebuf_peek){
	int n;


	n = 0;

	/* normal read */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(PEEK(2), 2);
	n += TEST_STRN_EQ(readb, "de", 2);
	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_STRN_EQ(readb, "de", 2);

	n += TEST_INT_EQ(PEEK(6), 6);
	n += TEST_STRN_EQ(readb, "adbeef", 6);
	n += TEST_INT_EQ(READ(6), 6);
	n += TEST_STRN_EQ(readb, "adbeef", 6);

	/* read empty buffer */
	INIT();

	n += TEST_INT_EQ(PEEK(2), 0);

	/* read more than data are available */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);

	n += TEST_INT_EQ(PEEK(100), 8);
	n += TEST_STRN_EQ(readb, "deadbeef", 8);

	return -n;
}

TEST(linebuf_write){
	int n;


	n = 0;

	/* normal write */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);
	n += TEST_STRN_EQ(buf.data, "deadbeef", 8);

	/* write entire buffer */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef0124"), 10);
	n += TEST_STRN_EQ(buf.data, "deadbeef01", 10);

	return -n;
}

TEST(linebuf_prefilled){
	int n;


	n = 0;

	/* init */
	strcpy(data, "dead");
	linebuf_init(&buf, data, 10, 4);

	n += TEST_INT_EQ(linebuf_contains(&buf), 4);
	n += TEST_INT_EQ(linebuf_left(&buf), 6);

	/* read */
	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_STRN_EQ(readb, "de", 2);
	n += TEST_INT_EQ(READ(4), 2);
	n += TEST_STRN_EQ(readb, "ad", 2);

	return -n;
}

TEST(linebuf_contains_left){
	int n;


	n = 0;

	/* empty */
	INIT();

	n += TEST_INT_EQ(linebuf_contains(&buf), 0);
	n += TEST_INT_EQ(linebuf_left(&buf), 10);

	/* partial filled */
	INIT();

	n += TEST_INT_EQ(WRITE("dead"), 4);
	n += TEST_INT_EQ(linebuf_contains(&buf), 4);
	n += TEST_INT_EQ(linebuf_left(&buf), 6);

	n += TEST_INT_EQ(READ(2), 2);
	n += TEST_INT_EQ(linebuf_contains(&buf), 2);
	n += TEST_INT_EQ(linebuf_left(&buf), 6);

	/* entirely filled */
	INIT();

	n += TEST_INT_EQ(WRITE("deadbeef0134"), 10);
	n += TEST_INT_EQ(linebuf_contains(&buf), 10);
	n += TEST_INT_EQ(linebuf_left(&buf), 0);

	return -n;
}

TEST(linebuf_empty){
	int n;


	n = 0;

	INIT();

	/* initially empty */
	n += TEST_INT_EQ(linebuf_empty(&buf), true);

	/* non-empty */
	n += TEST_INT_EQ(WRITE("dead"), 4);
	n += TEST_INT_EQ(linebuf_empty(&buf), false);

	/* still not empty */
	n += TEST_INT_EQ(READ(3), 3);
	n += TEST_INT_EQ(linebuf_empty(&buf), false);

	/* empty again */
	n += TEST_INT_EQ(READ(1), 1);
	n += TEST_INT_EQ(linebuf_empty(&buf), true);

	return -n;
}

TEST(linebuf_full){
	int n;


	n = 0;

	INIT();

	n += TEST_INT_EQ(linebuf_full(&buf), false);

	n += TEST_INT_EQ(WRITE("deadbeef"), 8);
	n += TEST_INT_EQ(linebuf_full(&buf), false);

	n += TEST_INT_EQ(WRITE("01"), 2);
	n += TEST_INT_EQ(linebuf_full(&buf), true);

	return -n;
}
