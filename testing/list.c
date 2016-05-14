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
typedef struct list_t{
	int el;
	char s[5];
	struct list_t *prev,
				  *next;
} list_t;


/* static variables */
static list_t el0 = { .el = 0, .s = "0" },
			  el1 = { .el = 1, .s = "1" },
			  el2 = { .el = 2, .s = "2" },
			  el3 = { .el = 3, .s = "3xx" };


/* local functions */
static int tc_list_print(int log){
	tlog(log, "el0 addr: %#x\n", &el0);
	tlog(log, "el1 addr: %#x\n", &el1);
	tlog(log, "el2 addr: %#x\n", &el2);
	tlog(log, "el3 addr: %#x\n", &el3);

	return 0;
}

test_case(tc_list_print, "list macros: addresses");


static int tc_list_empty(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	n += check_int(log, list_empty(head), true);

	list_add_head(&head, &el0);

	n += check_int(log, list_empty(head), false);

	return -n;
}

test_case(tc_list_empty, "list_empty");


static int tc_list_first_last(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_head(&head, &el3);

	n += check_ptr(log, list_first(head), &el3);
	n += check_ptr(log, list_last(head), &el3);

	return -n;
}

test_case(tc_list_first_last, "list_first/last");


static int tc_list_add_head(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_head(&head, &el0);
	list_add_head(&head, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el0);
	n += check_ptr(log, el1.next, &el0);
	n += check_ptr(log, el0.prev, &el1);
	n += check_ptr(log, el0.next, 0);

	return -n;
}

test_case(tc_list_add_head, "list_add_head");


static int tc_list_add_tail(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el1);
	n += check_ptr(log, el0.next, &el1);
	n += check_ptr(log, el1.prev, &el0);
	n += check_ptr(log, el1.next, 0);

	return -n;
}

test_case(tc_list_add_tail, "list_add_tail");


static int tc_list_add_in(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_add_in(&head, &el2, &el0, el0.next);

	n += check_ptr(log, el1.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, &el1);

	return -n;
}

test_case(tc_list_add_in, "list_add_in");


static int tc_list_replace(int log){
	unsigned int n;
	list_t *head;


	n = 0;

	// replace within empty list
	head = 0;
	INIT_EL();

	list_replace(&head, &el0, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el1);
	n += check_ptr(log, el1.next, 0);

	// replace head
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_replace(&head, &el0, &el1);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, el1.prev, &el1);
	n += check_ptr(log, el1.next, 0);

	// replace tail
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_replace(&head, &el1, &el2);

	n += check_ptr(log, list_last(head), &el2);
	n += check_ptr(log, el0.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, 0);

	// replace middle
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_add_tail(&head, &el2);
	list_replace(&head, &el1, &el3);

	n += check_ptr(log, el0.next, &el3);
	n += check_ptr(log, el2.prev, &el3);
	n += check_ptr(log, el3.prev, &el0);
	n += check_ptr(log, el3.next, &el2);

	return -n;
}

test_case(tc_list_replace, "list_replace");


static int tc_list_rm(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_add_tail(&head, &el2);

	// remove middle element
	list_rm(&head, &el1);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el2);
	n += check_ptr(log, el0.next, &el2);
	n += check_ptr(log, el2.prev, &el0);
	n += check_ptr(log, el2.next, 0);

	// remove tail element
	list_rm(&head, &el2);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, el0.prev, &el0);
	n += check_ptr(log, el0.next, 0);

	// remove head element
	list_rm(&head, &el0);

	n += check_ptr(log, head, 0);

	return -n;
}

test_case(tc_list_rm, "list_rm");


static int tc_list_find(int log){
	unsigned int n;
	list_t *head;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_add_tail(&head, &el2);
	list_add_tail(&head, &el3);

	/* list_find() */
	n += check_ptr(log, list_find(head, el, 2), &el2);
	n += check_ptr(log, list_find(head, el, 0), &el0);
	n += check_ptr(log, list_find(head, el, 1), &el1);

	/* list_find_str() */
	n += check_ptr(log, list_find_str(head, s, "2"), &el2);
	n += check_ptr(log, list_find_str(head, s, "0"), &el0);
	n += check_ptr(log, list_find_str(head, s, "3"), 0);

	/* list_find_strn() */
	n += check_ptr(log, list_find_strn(head, s, "2", 1), &el2);
	n += check_ptr(log, list_find_strn(head, s, "0", 1), &el0);
	n += check_ptr(log, list_find_strn(head, s, "3", 1), &el3);

	return -n;
}

test_case(tc_list_find, "list_find");


static int tc_list_for_each(int log){
	unsigned int n;
	list_t *head,
		   *el;


	n = 0;
	head = 0;
	INIT_EL();

	list_add_tail(&head, &el0);
	list_add_tail(&head, &el1);
	list_add_tail(&head, &el2);
	list_add_tail(&head, &el3);

	list_for_each(head, el)
		list_rm(&head, el);

	n += check_int(log, list_empty(head), true);

	return -n;
}

test_case(tc_list_for_each, "list_for_each");
