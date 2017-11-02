#include <arch/core.h>
#include <stdio.h>
#include <unistd.h>


/* global functions */
int main(void){
	size_t n;
	char buf[20];
	FILE *fp;


	fp = fopen("/dev/tty0", "r");

	if(fp == 0)
		goto idle;

	printf("start:\n");

	while(1){
//		n = fread(buf, 1, fp);
		n = read(3, buf, 1);

		if(errno){
			printf("error: %#x\n", errno);
			errno = E_OK;
			continue;
		}

		if(n == 0)
			continue;

		buf[n] = 0;
		printf("read: \"%s\" %u\n", buf, n);
	}


idle:
	while(1){
		core_sleep();
	}
}
