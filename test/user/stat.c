/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <test/test.h>


/* local functions */
/**
 * \brief	test to verify stat()
 */
TEST(stat){
	int r = 0;
	FILE *fp;
	stat_t s;


	/* prepare test file */
	unlink("dummy");

	r += TEST_PTR_NEQ(fp = fopen("dummy", "w"), 0x0);
	r += TEST_INT_EQ(fwrite("foo", 3, fp), 3);
	r += TEST_INT_EQ(fclose(fp), 0);

	/* check stat of test file */
	r += TEST_INT_EQ(stat("dummy", &s), 0);
	r += TEST_INT_EQ(s.type, FT_REG);
	r += TEST_INT_EQ(s.size, 3);

	/* check stat of /dev/tty0 */
	r += TEST_INT_EQ(stat("/dev/tty0", &s), 0);
	r += TEST_INT_EQ(s.type, FT_CHR);

	/* cleanup */
	r += TEST_INT_EQ(unlink("dummy"), 0);

	return -r;
}
