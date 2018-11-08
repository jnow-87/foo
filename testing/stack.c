/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/stack.h>
#include <sys/types.h>
#include <sys/string.h>
#include <testing/testcase.h>


/* macros */
#define INIT_EL() \
	el0.next = (void*)0x1; \
	el1.next = (void*)0x1; \
	el2.next = (void*)0x1; \
	el3.next = (void*)0x1; \


/* types */
typedef struct stack_t{
	struct stack_t *next;

	int el;
	char s[5];
} stack_t;


/* static variables */
static stack_t el0 = { .el = 0, .s = "0" },
			   el1 = { .el = 1, .s = "1" },
			   el2 = { .el = 2, .s = "2" },
			   el3 = { .el = 3, .s = "3xx" };


/* local functions */
static int tc_stack_print(int log){
	tlog(log, "list element addresses\n");
	tlog(log, "el0 addr: %#x\n", &el0);
	tlog(log, "el1 addr: %#x\n", &el1);
	tlog(log, "el2 addr: %#x\n", &el2);
	tlog(log, "el3 addr: %#x\n", &el3);

	return 0;
}

test_case(tc_stack_print, "stack_print");


static int tc_stack_init(int log){
	unsigned int n;
	stack_t *top;


	n = 0;
	top = &el0;

	stack_init(top);

	n += check_ptr(log, top->next, 0);

	return -n;
}

test_case(tc_stack_init, "stack_init");


static int tc_stack_push(int log){
	unsigned int n;
	stack_t *top;


	n = 0;
	top = 0;
	INIT_EL();

	// push to empty stack
	stack_push(top, &el0);

	n += check_ptr(log, top, &el0);
	n += check_ptr(log, el0.next, 0);

	// push to non-empty stack
	stack_push(top, &el1);

	n += check_ptr(log, top, &el1);
	n += check_ptr(log, el1.next, &el0);
	n += check_ptr(log, el0.next, 0);

	return -n;
}

test_case(tc_stack_push, "stack_push");


static int tc_stack_pop(int log){
	unsigned int n;
	stack_t *top,
			*el;


	n = 0;
	top = 0;
	INIT_EL();

	// prepare
	stack_push(top, &el0);
	stack_push(top, &el1);

	// pop from non-empty stack
	el = stack_pop(top);

	n += check_ptr(log, top, &el0);
	n += check_ptr(log, el, &el1);

	// pop last element
	el = stack_pop(top);

	n += check_ptr(log, top, 0);
	n += check_ptr(log, el, &el0);

	// pop from empty stack
	el = stack_pop(top);

	n += check_ptr(log, top, 0);
	n += check_ptr(log, el, 0);

	return -n;
}

test_case(tc_stack_pop, "stack_pop");
