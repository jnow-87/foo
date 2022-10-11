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
#define INIT()		linebuf_init(&line, buf, 10, 0)
#define READ(n)		linebuf_read(&line, readb, n)
#define PEEK(n)		linebuf_peek(&line, readb, n)
#define WRITE(s)	linebuf_write(&line, s, strlen(s))


/* static variables */
static linebuf_t line;
static char buf[10];
static char readb[10];


/* local functions */
TEST(linebuf_reset){
	int r = 0;


	/* no change to empty buffer */
	INIT();

	r += TEST_INT_EQ(linebuf_contains(&line), 0);
	r += TEST_INT_EQ(linebuf_left(&line), 10);

	linebuf_reset(&line);
	r += TEST_INT_EQ(linebuf_contains(&line), 0);
	r += TEST_INT_EQ(linebuf_left(&line), 10);

	/* reset after write */
	INIT();

	r += TEST_INT_EQ(WRITE("dead"), 4);
	r += TEST_INT_EQ(linebuf_contains(&line), 4);
	r += TEST_INT_EQ(linebuf_left(&line), 6);

	linebuf_reset(&line);
	r += TEST_INT_EQ(linebuf_contains(&line), 0);
	r += TEST_INT_EQ(linebuf_left(&line), 10);

	/* reset after write + read */
	INIT();

	r += TEST_INT_EQ(WRITE("dead"), 4);
	r += TEST_INT_EQ(READ(2), 2);
	r += TEST_INT_EQ(linebuf_contains(&line), 2);
	r += TEST_INT_EQ(linebuf_left(&line), 6);

	linebuf_reset(&line);
	r += TEST_INT_EQ(linebuf_contains(&line), 0);
	r += TEST_INT_EQ(linebuf_left(&line), 10);

	return -r;
}

TEST(linebuf_read){
	int r = 0;


	/* normal read */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);

	r += TEST_INT_EQ(READ(2), 2);
	r += TEST_STRN_EQ(readb, "de", 2);
	r += TEST_INT_EQ(READ(6), 6);
	r += TEST_STRN_EQ(readb, "adbeef", 6);

	/* read empty buffer */
	INIT();

	r += TEST_INT_EQ(READ(2), 0);

	/* read more than data are available */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);

	r += TEST_INT_EQ(READ(100), 8);
	r += TEST_STRN_EQ(readb, "deadbeef", 8);

	return -r;
}

TEST(linebuf_peek){
	int r = 0;


	/* normal read */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);

	r += TEST_INT_EQ(PEEK(2), 2);
	r += TEST_STRN_EQ(readb, "de", 2);
	r += TEST_INT_EQ(READ(2), 2);
	r += TEST_STRN_EQ(readb, "de", 2);

	r += TEST_INT_EQ(PEEK(6), 6);
	r += TEST_STRN_EQ(readb, "adbeef", 6);
	r += TEST_INT_EQ(READ(6), 6);
	r += TEST_STRN_EQ(readb, "adbeef", 6);

	/* read empty buffer */
	INIT();

	r += TEST_INT_EQ(PEEK(2), 0);

	/* read more than data are available */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);

	r += TEST_INT_EQ(PEEK(100), 8);
	r += TEST_STRN_EQ(readb, "deadbeef", 8);

	return -r;
}

TEST(linebuf_write){
	int r = 0;


	/* normal write */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);
	r += TEST_STRN_EQ(line.buf, "deadbeef", 8);

	/* write entire buffer */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef0124"), 10);
	r += TEST_STRN_EQ(line.buf, "deadbeef01", 10);

	return -r;
}

TEST(linebuf_prefilled){
	int r = 0;


	/* init */
	strcpy(buf, "dead");
	linebuf_init(&line, buf, 10, 4);

	r += TEST_INT_EQ(linebuf_contains(&line), 4);
	r += TEST_INT_EQ(linebuf_left(&line), 6);

	/* read */
	r += TEST_INT_EQ(READ(2), 2);
	r += TEST_STRN_EQ(readb, "de", 2);
	r += TEST_INT_EQ(READ(4), 2);
	r += TEST_STRN_EQ(readb, "ad", 2);

	return -r;
}

TEST(linebuf_contains_left){
	int r = 0;


	/* empty */
	INIT();

	r += TEST_INT_EQ(linebuf_contains(&line), 0);
	r += TEST_INT_EQ(linebuf_left(&line), 10);

	/* partial filled */
	INIT();

	r += TEST_INT_EQ(WRITE("dead"), 4);
	r += TEST_INT_EQ(linebuf_contains(&line), 4);
	r += TEST_INT_EQ(linebuf_left(&line), 6);

	r += TEST_INT_EQ(READ(2), 2);
	r += TEST_INT_EQ(linebuf_contains(&line), 2);
	r += TEST_INT_EQ(linebuf_left(&line), 6);

	/* entirely filled */
	INIT();

	r += TEST_INT_EQ(WRITE("deadbeef0134"), 10);
	r += TEST_INT_EQ(linebuf_contains(&line), 10);
	r += TEST_INT_EQ(linebuf_left(&line), 0);

	return -r;
}

TEST(linebuf_empty){
	int r = 0;


	INIT();

	/* initially empty */
	r += TEST_INT_EQ(linebuf_empty(&line), true);

	/* non-empty */
	r += TEST_INT_EQ(WRITE("dead"), 4);
	r += TEST_INT_EQ(linebuf_empty(&line), false);

	/* still not empty */
	r += TEST_INT_EQ(READ(3), 3);
	r += TEST_INT_EQ(linebuf_empty(&line), false);

	/* empty again */
	r += TEST_INT_EQ(READ(1), 1);
	r += TEST_INT_EQ(linebuf_empty(&line), true);

	return -r;
}

TEST(linebuf_full){
	int r = 0;


	INIT();

	r += TEST_INT_EQ(linebuf_full(&line), false);

	r += TEST_INT_EQ(WRITE("deadbeef"), 8);
	r += TEST_INT_EQ(linebuf_full(&line), false);

	r += TEST_INT_EQ(WRITE("01"), 2);
	r += TEST_INT_EQ(linebuf_full(&line), true);

	return -r;
}
