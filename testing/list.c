/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/types.h>
#include <sys/string.h>
#include <testing/testcase.h>


/* macros */
#define INIT_EL() \
	el0.prev = el0.next = (void*)0x1; \
	el1.prev = el1.next = (void*)0x1; \
	el2.prev = el2.next = (void*)0x1; \
	el3.prev = el3.next = (void*)0x1; \


/* types */
typedef struct tlist_t{
	struct tlist_t *prev,
				   *next;

	int el;
	char const *s_ptr;
	char s_arr[5];
} tlist_t;


/* static variables */
static char const el_names[][5] = {
	"0",
	"1",
	"2",
	"3xx",
};

static tlist_t el0 = { .el = 0, .s_ptr = el_names[0], .s_arr = "0" },
			   el1 = { .el = 1, .s_ptr = el_names[1], .s_arr = "1" },
			   el2 = { .el = 2, .s_ptr = el_names[2], .s_arr = "2" },
			   el3 = { .el = 3, .s_ptr = el_names[3], .s_arr = "3xx" };


/* local functions */
static int tc_list_print(int log){
	tlog(log, "list element addresses\n");
	tlog(log, "el0 addr: %#x\n", &el0);
	tlog(log, "el1 addr: %#x\n", &el1);
	tlog(log, "el2 addr: %#x\n", &el2);
	tlog(log, "el3 addr: %#x\n", &el3);

	return 0;
}

test_case(tc_list_print, "list_print");


static int tc_list_init(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = &el0;

	list_init(head);

	n += check_ptr(log, head->prev, head);
	n += check_ptr(log, head->next, 0x0);

	return -n;
}

test_case(tc_list_init, "list_init");


static int tc_list_empty(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	n += check_int(log, list_empty(head), true);

	list_add_head(head, &el0);

	n += check_int(log, list_empty(head), false);

	return -n;
}

test_case(tc_list_empty, "list_empty");


static int tc_list_first_last(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_head(head, &el3);

	n += check_ptr(log, list_first(head), &el3);
	n += check_ptr(log, list_last(head), &el3);

	return -n;
}

test_case(tc_list_first_last, "list_first/last");


static int tc_list_add_head(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_head(head, &el0);
	list_add_head(head, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el0);
	n += check_ptr(log, el1.next, &el0);
	n += check_ptr(log, el0.prev, &el1);
	n += check_ptr(log, el0.next, 0x0);

	return -n;
}

test_case(tc_list_add_head, "list_add_head");


static int tc_list_add_tail(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el1);
	n += check_ptr(log, el0.next, &el1);
	n += check_ptr(log, el1.prev, &el0);
	n += check_ptr(log, el1.next, 0x0);

	return -n;
}

test_case(tc_list_add_tail, "list_add_tail");


static int tc_list_add_in(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_in(&el2, &el0, el0.next);

	n += check_ptr(log, el1.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, &el1);

	return -n;
}

test_case(tc_list_add_in, "list_add_in");


static int tc_list_replace(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;

	// replace within empty list
	head = 0x0;
	INIT_EL();

	list_replace(head, &el0, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el1);
	n += check_ptr(log, el1.next, 0x0);

	// replace head
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_replace(head, &el0, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el1);
	n += check_ptr(log, el1.next, 0x0);

	// replace tail
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_replace(head, &el1, &el2);

	n += check_ptr(log, list_last(head), &el2);
	n += check_ptr(log, el0.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, 0x0);

	// replace middle
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_replace(head, &el1, &el3);

	n += check_ptr(log, el0.next, &el3);
	n += check_ptr(log, el2.prev, &el3);
	n += check_ptr(log, el3.prev, &el0);
	n += check_ptr(log, el3.next, &el2);

	return -n;
}

test_case(tc_list_replace, "list_replace");


static int tc_list_rm(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);

	// remove middle element
	list_rm(head, &el1);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, 0x0);

	// remove tail element
	list_rm(head, &el2);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el0);
	n += check_ptr(log, el0.next, 0x0);

	// remove head element
	list_rm(head, &el0);

	n += check_ptr(log, head, 0x0);

	return -n;
}

test_case(tc_list_rm, "list_rm");


static int tc_list_contains(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);

	n += check_int(log, list_contains(head, &el1), true);
	n += check_int(log, list_contains(head, &el3), false);

	return -n;
}

test_case(tc_list_contains, "list_contains");


static int tc_list_find(int log){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_add_tail(head, &el3);

	/* list_find() */
	n += check_ptr(log, list_find(head, el, 2), &el2);
	n += check_ptr(log, list_find(head, el, 0), &el0);
	n += check_ptr(log, list_find(head, el, 1), &el1);

	/* list_find_str() with pointer target */
	n += check_ptr(log, list_find_str(head, s_ptr, "2"), &el2);
	n += check_ptr(log, list_find_str(head, s_ptr, "0"), &el0);
	n += check_ptr(log, list_find_str(head, s_ptr, "3"), 0x0);

	/* list_find_strn() with pointer target */
	n += check_ptr(log, list_find_strn(head, s_ptr, "2", 1), &el2);
	n += check_ptr(log, list_find_strn(head, s_ptr, "0", 1), &el0);
	n += check_ptr(log, list_find_strn(head, s_ptr, "3", 1), &el3);

	/* list_find_str() with array target */
	n += check_ptr(log, list_find_str(head, s_arr, "2"), &el2);
	n += check_ptr(log, list_find_str(head, s_arr, "0"), &el0);
	n += check_ptr(log, list_find_str(head, s_arr, "3"), 0x0);

	/* list_find_strn() with array target */
	n += check_ptr(log, list_find_strn(head, s_arr, "2", 1), &el2);
	n += check_ptr(log, list_find_strn(head, s_arr, "0", 1), &el0);
	n += check_ptr(log, list_find_strn(head, s_arr, "3", 1), &el3);

	return -n;
}

test_case(tc_list_find, "list_find");


static int tc_list_for_each(int log){
	unsigned int n;
	tlist_t *head,
		   *el;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_add_tail(head, &el3);

	list_for_each(head, el)
		list_rm(head, el);

	n += check_int(log, list_empty(head), true);

	return -n;
}

test_case(tc_list_for_each, "list_for_each");
