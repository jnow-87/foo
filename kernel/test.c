/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/kprintf.h>
#include <kernel/opt.h>
#include <sys/stdarg.h>
#include <sys/escape.h>
#include <test/test.h>


/* global functions */
void ktest(){
	size_t failed = 0;
	int r;
	test_t *test;


	kprintf(KMSG_ANY, "execute kernel tests\n");

	test_for_each_type(test, kernel){
		kprintf(KMSG_ANY, " %s...%c", test->name, (kopt.verbose_test ? '\n' : ' '));

		r = (test->call() != 0);
		failed += r;

		kprintf(KMSG_ANY, "%s%s" RESET_ATTR "\n",
			((char const *[]){ FG_GREEN, FG_RED }[r]),
			((char const *[]){ "passed", "failed" }[r])
		);
	}

	kprintf(KMSG_ANY, " tests " FG("failed", RED) ": %u\n", failed);
}

void test_log(char const *fmt, ...){
	va_list lst;


	if(!kopt.verbose_test)
		return;

	va_start(lst, fmt);
	kvprintf(KMSG_ANY, fmt, lst);
	va_end(lst);
}
